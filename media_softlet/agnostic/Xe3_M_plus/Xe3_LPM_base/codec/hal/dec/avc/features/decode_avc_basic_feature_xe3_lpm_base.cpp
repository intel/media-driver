/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     decode_avc_basic_feature_xe3_lpm_base.cpp
//! \brief    Defines the common interface for decode avc parameter
//!

#include "decode_avc_basic_feature_xe3_lpm_base.h"

namespace decode
{
    MOS_STATUS AvcBasicFeatureXe3_Lpm_Base::CheckBitDepthAndChromaSampling()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_avcPicParams);

        bool is8bit  = m_avcPicParams->bit_depth_luma_minus8 == 0 && m_avcPicParams->bit_depth_chroma_minus8 == 0;
        bool is10bit = m_avcPicParams->bit_depth_luma_minus8 == 2 && m_avcPicParams->bit_depth_chroma_minus8 == 2;

        if (!is8bit && !is10bit)
        {
            DECODE_ASSERTMESSAGE("Only 8/10bit decode is supported!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_avcPicParams->seq_fields.chroma_format_idc == avcChromaFormatMono && is10bit)
        {
            DECODE_ASSERTMESSAGE("AVC 4:0:0 10bit decoding is not supported!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_avcPicParams->seq_fields.chroma_format_idc == avcChromaFormat422 && is8bit)
        {
            DECODE_ASSERTMESSAGE("AVC 4:2:2 8bit decoding is not supported!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_avcPicParams->seq_fields.chroma_format_idc == avcChromaFormat444)
        {
            DECODE_ASSERTMESSAGE("AVC 4:4:4 decoding is not supported!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (is10bit)
        {
            // AVC 10bit decoding driver check
            if (!m_avcPicParams->seq_fields.frame_mbs_only_flag)
            {
                DECODE_ASSERTMESSAGE("AVC 10 bit decoding does not support interlace mode!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            if (m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag && !m_avcPicParams->pic_fields.field_pic_flag)
            {
                DECODE_ASSERTMESSAGE("AVC 10 bit decoding does not support MBAFF!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            if (m_avcPicParams->num_slice_groups_minus1 != 0)
            {
                DECODE_ASSERTMESSAGE("AVC 10 bit decoding does not support slice group!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            if (m_avcPicParams->pic_fields.redundant_pic_cnt_present_flag)
            {
                DECODE_ASSERTMESSAGE("AVC 10 bit decoding does not support redundant picture!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
