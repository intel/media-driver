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
//! \file     decode_mpeg2_mb_packet_xe3_lpm_base.h
//! \brief    Defines the implementation of mpeg2 decode macroblock packet for Xe3_LPM+
//!

#ifndef __DECODE_MPEG2_MB_PACKET_XE3_LPM_BASE_H__
#define __DECODE_MPEG2_MB_PACKET_XE3_LPM_BASE_H__

#include "media_cmd_packet.h"
#include "decode_mpeg2_pipeline.h"
#include "decode_utils.h"
#include "decode_mpeg2_basic_feature.h"
#include "decode_mpeg2_mb_packet.h"

namespace decode
{
    class Mpeg2DecodeMbPktXe3_Lpm_Base : public Mpeg2DecodeMbPkt
    {
    public:
        Mpeg2DecodeMbPktXe3_Lpm_Base(Mpeg2Pipeline *pipeline, CodechalHwInterfaceNext* hwInterface)
            : Mpeg2DecodeMbPkt(pipeline, hwInterface)
        {
        }

        virtual ~Mpeg2DecodeMbPktXe3_Lpm_Base() {};
        virtual MOS_STATUS Execute(MHW_BATCH_BUFFER& batchBuffer, uint32_t mbIdx) override;

    private:
        uint16_t m_lastMbAddress = 0;  //!< Address of last macro block

    MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodeMbPktXe3_Lpm_Base)
    };

}  // namespace decode

#endif  // __DECODE_MPEG2_MB_PACKET_XE3_LPM_BASE_H__
