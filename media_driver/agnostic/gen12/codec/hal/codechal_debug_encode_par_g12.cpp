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
//! \file     codechal_debug_encode_par_g12.cpp
//! \brief    Implements the debug interface shared by all of CodecHal for encode
//!           PAR file dump.
//!

#include "codechal_debug.h"
#if USE_CODECHAL_DEBUG_TOOL

#include "codechal_debug_encode_par_g12.h"
#include "codechal_encode_csc_ds_g12.h"
#include "codechal_kernel_hme_g12.h"

MOS_STATUS CodechalDebugEncodeParG12::PopulateDsParam(void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(m_commonPar);

    CodechalEncodeCscDsG12::Ds4xKernelCurbeData *curbe = (CodechalEncodeCscDsG12::Ds4xKernelCurbeData *)cmd;

    if ((m_encoder->m_pictureCodingType == I_TYPE) && (curbe->DW6_EnableMBFlatnessCheck))
    {
        m_commonPar->mbFlatnessThreshold = curbe->DW5_FlatnessThreshold;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugEncodeParG12::PopulateHmeParam(
    bool    is16xMeEnabled,
    bool    is32xMeEnabled,
    uint8_t meMethod,
    void    *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(m_commonPar);

    CodechalKernelHmeG12::Curbe *curbe = (CodechalKernelHmeG12::Curbe *)cmd;

    if (m_encoder->m_pictureCodingType == P_TYPE)
    {
        m_commonPar->superHME         = is16xMeEnabled;
        m_commonPar->ultraHME         = is32xMeEnabled;
        m_commonPar->superCombineDist = curbe->m_data.DW6.SuperCombineDist;

        if (m_encoder->m_vdencEnabled)
        {
            m_commonPar->streamInEnable       = is16xMeEnabled;
            m_commonPar->streamInL0FromNewRef = is16xMeEnabled ? 7 : 0;
            m_commonPar->streamInL1FromNewRef = 0;
            m_commonPar->meMethod             = meMethod;
        }
    }

    return MOS_STATUS_SUCCESS;
}

#endif // USE_CODECHAL_DEBUG_TOOL
