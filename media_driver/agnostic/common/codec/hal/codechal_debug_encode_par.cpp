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
//! \file     codechal_debug_encode_par.cpp
//! \brief    Implements the debug interface shared by all of CodecHal for encode
//!           PAR file dump.
//!

#include "codechal_debug.h"
#if USE_CODECHAL_DEBUG_TOOL

#include "codechal_debug_encode_par.h"

MOS_STATUS CodechalDebugEncodePar::Initialize()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(m_commonPar = MOS_New(EncodeCommonPar));
    MOS_ZeroMemory(m_commonPar, sizeof(EncodeCommonPar));

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    // Need to fill manually
    oss << "SourceFile = " << std::endl;
    oss << "DstFile = " << std::endl;
    oss << "NumFrames = " << std::endl;
    oss << "SourceWidth = " << std::endl;
    oss << "SourceHeight = " << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "EncodeSequence",
        "EncodePar",
        CodechalDbgExtType::par);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugEncodePar::Destroy()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (m_commonPar)
    {
        MOS_Delete(m_commonPar);
        m_commonPar = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

CodechalDebugEncodePar::CodechalDebugEncodePar(
    CodechalEncoderState *encoder)
{
    m_encoder        = encoder;
    m_osInterface    = encoder->GetOsInterface();
    m_hwInterface    = encoder->GetHwInterface();
    m_debugInterface = encoder->GetDebugInterface();

    Initialize();
}

CodechalDebugEncodePar::~CodechalDebugEncodePar()
{
    Destroy();
}

#endif // USE_CODECHAL_DEBUG_TOOL
