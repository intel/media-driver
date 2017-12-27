/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_vdenc_avc_g9.cpp
//! \brief    This file implements the C++ class/interface for Gen9 platform's AVC 
//!           VDEnc encoding to be used CODECHAL components.
//!

#include "codechal_vdenc_avc_g9.h"
#include "codeckrnheader.h"
#include "igcodeckrn_g9.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g9.h"
#endif

CodechalVdencAvcStateG9::CodechalVdencAvcStateG9(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalVdencAvcState(hwInterface, debugInterface, standardInfo)
{
    m_kernelBase = (uint8_t*)IGCODECKRN_G9;
    m_kuid = IDR_CODEC_AllAVCEnc;
    AddIshSize(m_kuid, m_kernelBase);
    
    m_cmKernelEnable = true;
    m_mbStatsSupported = true; //Starting from GEN9

    m_needCheckCpEnabled = true;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_encodeParState = MOS_New(CodechalDebugEncodeParG9, this));
        CreateAvcPar();
    )
}

CodechalVdencAvcStateG9::~CodechalVdencAvcStateG9()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(
        DestroyAvcPar();
        MOS_Delete(m_encodeParState);
    )
}
