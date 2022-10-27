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
//! \file     decode_avc_slice_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for avc decode slice packet for Xe_LPM_plus+
//!
#include "decode_avc_slice_packet_xe_lpm_plus_base.h"

namespace decode
{

    MOS_STATUS AvcDecodeSlcPktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t slcIdx)
    {
        m_curSliceNum = slcIdx;
        //AVC Slice Level Commands
        if (m_avcPipeline->IsShortFormat())
        {
            DECODE_CHK_STATUS(AddCmd_AVC_SLICE_Addr(cmdBuffer, slcIdx));
        }
        else
        {
            PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;

            // Add phantom slice command if necessary
            if (m_firstValidSlice && slc->first_mb_in_slice)
            {
                // ensure that slc->first_mb_in_next_slice is always non-zero for this phantom slice
                uint16_t nextStartMbNum = slc->first_mb_in_next_slice;
                uint16_t startMbNum = slc->first_mb_in_slice;
                slc->first_mb_in_slice = 0;
                slc->first_mb_in_next_slice = startMbNum;

                DECODE_CHK_STATUS(AddCmd_AVC_PHANTOM_SLICE(cmdBuffer, slcIdx));
                DECODE_CHK_STATUS(AddCmd_AVC_BSD_OBJECT(cmdBuffer, slcIdx));

                slc->first_mb_in_slice = startMbNum;
                slc->first_mb_in_next_slice = nextStartMbNum;
            }

            m_firstValidSlice = false;

            if (!m_avcBasicFeature->IsAvcISlice(slc->slice_type))
            {
                DECODE_CHK_STATUS(AddCmd_AVC_SLICE_REF_IDX(cmdBuffer, slcIdx));
                DECODE_CHK_STATUS(AddCmd_AVC_SLICE_WEIGHT_OFFSET(cmdBuffer, slcIdx));
            }
            else if (m_avcBasicFeature->m_useDummyReference && !m_osInterface->bSimIsActive)
            {
                // set dummy reference for I Frame
                DECODE_CHK_STATUS(AddCmd_AVC_SLICE_REF_IDX(cmdBuffer, 0));
            }

            DECODE_CHK_STATUS(AddCmd_AVC_SLICE_STATE(cmdBuffer, slcIdx));
        }
        DECODE_CHK_STATUS(AddCmd_AVC_BSD_OBJECT(cmdBuffer, slcIdx));

        return MOS_STATUS_SUCCESS;
    }

}
