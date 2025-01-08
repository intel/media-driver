/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_mpeg2_mb_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for mpeg2 decode macroblock packet for Xe3_LPM+
//!

#include "decode_mpeg2_mb_packet_xe3_lpm_base.h"

namespace decode
{
    MOS_STATUS Mpeg2DecodeMbPktXe3_Lpm_Base::Execute(MHW_BATCH_BUFFER& batchBuffer, uint32_t mbIdx)
    {
        DECODE_FUNC_CALL();

        if (m_mpeg2BasicFeature->m_mbRecord[mbIdx].skippedMBs)
        {
            DECODE_CHK_STATUS(AddAllCmdsInsertSkippedMacroblocks(
                batchBuffer,
                mbIdx,
                m_mpeg2BasicFeature->m_mbRecord[mbIdx].expectedMBAddr,
                m_mpeg2BasicFeature->m_mbRecord[mbIdx].skippedMBs));
        }

        DECODE_CHK_STATUS(AddCmd_MFD_MPEG2_IT_OBJECT(batchBuffer, mbIdx));

        if (m_mpeg2PicParams->m_pictureCodingType != I_TYPE &&
            m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam.m_mbSkipFollowing)
        {
            uint16_t skippedMBs = m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam.m_mbSkipFollowing;
            uint16_t skippedMBStart = m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam.m_mbAddr + 1;
            DECODE_CHK_STATUS(AddAllCmdsInsertSkippedMacroblocks(
                batchBuffer,
                mbIdx,
                m_mpeg2BasicFeature->m_mbRecord[mbIdx].expectedMBAddr,
                m_mpeg2BasicFeature->m_mbRecord[mbIdx].skippedMBs));
            m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam.m_mbAddr += skippedMBs;
        }

        //save the last MB's parameters for later use to insert skipped MBs.
        m_mpeg2BasicFeature->m_savedMpeg2MbParam = m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam;
        if (m_mpeg2BasicFeature->m_incompletePicture && mbIdx == (m_mpeg2BasicFeature->m_totalNumMbsRecv - 1))
        {
            uint16_t u16NextMBStart = m_mpeg2BasicFeature->m_savedMpeg2MbParam.m_mbAddr + 1;  // = 1 + saved last MB's address in this picture.
            uint16_t numMBs = (m_mpeg2BasicFeature->m_picWidthInMb * m_mpeg2BasicFeature->m_picWidthInMb) - u16NextMBStart;
            DECODE_CHK_STATUS(AddAllCmdsInsertSkippedMacroblocks(
                batchBuffer,
                mbIdx,
                m_mpeg2BasicFeature->m_mbRecord[mbIdx].expectedMBAddr,
                m_mpeg2BasicFeature->m_mbRecord[mbIdx].skippedMBs));
        }

        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
