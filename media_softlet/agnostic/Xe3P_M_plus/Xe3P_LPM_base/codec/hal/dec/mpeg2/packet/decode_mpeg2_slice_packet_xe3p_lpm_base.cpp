/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     decode_mpeg2_slice_packet_xe3p_lpm_base.cpp
//! \brief    Defines the interface for mpeg2 decode slice packet for Xe3P_LPM_Base
//!

#include "decode_mpeg2_slice_packet_xe3p_lpm_base.h"

namespace decode
{
    MOS_STATUS Mpeg2DecodeSlcPktXe3P_Lpm_Base::Execute(MHW_BATCH_BUFFER& batchBuffer, uint16_t slcIdx)
    {
        DECODE_FUNC_CALL();

        if (!m_mpeg2BasicFeature->m_sliceRecord[slcIdx].skip)
        {
            if (m_mpeg2BasicFeature->m_sliceRecord[slcIdx].sliceStartMbOffset !=
                m_mpeg2BasicFeature->m_sliceRecord[slcIdx].prevSliceMbEnd)
            {
                uint16_t startMB = (uint16_t)m_mpeg2BasicFeature->m_sliceRecord[slcIdx].prevSliceMbEnd;
                uint16_t endMB = (uint16_t)m_mpeg2BasicFeature->m_sliceRecord[slcIdx].sliceStartMbOffset;
                DECODE_CHK_STATUS(AddAllCmdsInsertDummySlice(batchBuffer, startMB, endMB));
            }
            DECODE_CHK_STATUS(AddCmd_MFD_MPEG2_BSD_OBJECT(batchBuffer, slcIdx));
        }

        if ((slcIdx == (m_mpeg2BasicFeature->m_totalNumSlicesRecv - 1) &&
            m_mpeg2BasicFeature->m_incompletePicture))
        {
            uint16_t lastMbAddress = m_mpeg2BasicFeature->m_lastMbAddress;
            uint16_t expectedMbEnd = m_mpeg2BasicFeature->m_picWidthInMb * m_mpeg2BasicFeature->m_picHeightInMb;
            DECODE_CHK_STATUS(AddAllCmdsInsertDummySlice(
                batchBuffer,
                lastMbAddress,
                expectedMbEnd));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeSlcPktXe3P_Lpm_Base::CalculateSliceStateCommandSize()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;
        DECODE_CHK_NULL(m_mfxItf);

        uint32_t maxSize = 0, patchListMaxSize = 0;

        if (m_mpeg2BasicFeature && m_mpeg2BasicFeature->m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
        {
            maxSize =
                m_mfxItf->MHW_GETSIZE_F(MFD_MPEG2_BSD_OBJECT)();

            patchListMaxSize =
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_MPEG2_BSD_OBJECT_CMD);
        }
        else if (m_mpeg2BasicFeature && m_mpeg2BasicFeature->m_mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
        {
            maxSize =
                m_mfxItf->MHW_GETSIZE_F(MFD_IT_OBJECT)() +
                m_mfxItf->MHW_GETSIZE_F(MFD_IT_OBJECT_MPEG2_INLINE_DATA)();

            patchListMaxSize =
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_MPEG2_IT_OBJECT_CMD);
        }

        m_sliceStatesSize    = maxSize;
        m_slicePatchListSize = patchListMaxSize;

        return eStatus;
    }

}  // namespace decode
