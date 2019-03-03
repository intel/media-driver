/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     codechal_decode_vc1_g9.cpp
//! \brief    Implements the decode interface extension for Gen9 VC1.
//! \details  Implements all functions required by CodecHal for Gen9 VC1 decoding.
//!

#include "codeckrnheader.h"
#include "igcodeckrn_g9.h"
#include "codechal_decode_vc1_g9.h"

CodechalDecodeVc1G9::CodechalDecodeVc1G9(
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecodeVc1(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);

    m_olpCurbeStaticDataLength = CODECHAL_DECODE_VC1_CURBE_SIZE_OLP;

    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        (uint8_t *)IGCODECKRN_G9,
        IDR_CODEC_AllVC1_NV12,
        &m_olpKernelBase,
        &m_olpKernelSize);
    CODECHAL_DECODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_DECODE_VC1_NUM_SYNC_TAGS;
    hwInterface->GetStateHeapSettings()->dwIshSize =
        MOS_ALIGN_CEIL(m_olpKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
    hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_DECODE_VC1_INITIAL_DSH_SIZE;
}
