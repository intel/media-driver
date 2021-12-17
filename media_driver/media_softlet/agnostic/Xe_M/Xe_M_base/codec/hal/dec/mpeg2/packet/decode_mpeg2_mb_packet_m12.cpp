/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_mpeg2_mb_packet_m12.cpp
//! \brief    Defines the interface for mpeg2 decode macroblock packet for GEN12
//!
#include "codechal_utilities.h"
#include "decode_mpeg2_mb_packet_m12.h"
#include "mhw_vdbox_mfx_g12_X.h"

namespace decode{

MOS_STATUS Mpeg2DecodeMbPktM12::Execute(MHW_BATCH_BUFFER& batchBuffer, uint32_t mbIdx)
{
    DECODE_FUNC_CALL();

    if (m_mpeg2BasicFeature->m_mbRecord[mbIdx].skippedMBs)
    {
        DECODE_CHK_STATUS(InsertSkippedMacroblocks(
                             batchBuffer,
                             mbIdx,
                             m_mpeg2BasicFeature->m_mbRecord[mbIdx].expectedMBAddr,
                             m_mpeg2BasicFeature->m_mbRecord[mbIdx].skippedMBs));
    }

    DECODE_CHK_STATUS(AddITObj(batchBuffer, mbIdx));

    if (m_mpeg2PicParams->m_pictureCodingType != I_TYPE &&
        m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam.m_mbSkipFollowing)
    {
        uint16_t skippedMBs    = m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam.m_mbSkipFollowing;
        uint16_t skippedMBStart = m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam.m_mbAddr + 1;
        DECODE_CHK_STATUS(InsertSkippedMacroblocks(
                             batchBuffer,
                             mbIdx,
                             skippedMBStart,
                             skippedMBs));
        m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam.m_mbAddr += skippedMBs;
    }

    //save the last MB's parameters for later use to insert skipped MBs.
    m_mpeg2BasicFeature->m_savedMpeg2MbParam = m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam;
    if (m_mpeg2BasicFeature->m_incompletePicture && mbIdx == (m_mpeg2BasicFeature->m_totalNumMbsRecv - 1))
    {
        uint16_t u16NextMBStart = m_mpeg2BasicFeature->m_savedMpeg2MbParam.m_mbAddr + 1; // = 1 + saved last MB's address in this picture.
        uint16_t numMBs = (m_mpeg2BasicFeature->m_picWidthInMb * m_mpeg2BasicFeature->m_picWidthInMb) - u16NextMBStart;
        DECODE_CHK_STATUS(InsertSkippedMacroblocks(
                        batchBuffer,
                        mbIdx,
                        u16NextMBStart,
                        numMBs));
    }

    return MOS_STATUS_SUCCESS;
}

}
