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
//! \file     decode_avc_slice_packet_xe3_lpm_base.h
//! \brief    Defines the implementation of avc decode slice packet for Xe3_LPM+
//!

#ifndef __DECODE_AVC_SLICE_PACKET_XE3_LPM_BASE_H__
#define __DECODE_AVC_SLICE_PACKET_XE3_LPM_BASE_H__

#include "media_cmd_packet.h"
#include "decode_avc_pipeline.h"
#include "decode_utils.h"
#include "decode_avc_basic_feature.h"
#include "decode_avc_slice_packet.h"

namespace decode
{
class AvcDecodeSlcPktXe3_Lpm_Base : public AvcDecodeSlcPkt
    {
    public:
        AvcDecodeSlcPktXe3_Lpm_Base(AvcPipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
            : AvcDecodeSlcPkt(pipeline, hwInterface)
        {
        }
        virtual ~AvcDecodeSlcPktXe3_Lpm_Base(){};

        MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t slcIdx) override;

    protected:

    MEDIA_CLASS_DEFINE_END(decode__AvcDecodeSlcPktXe3_Lpm_Base)
    };
}  // namespace decode
#endif  // !__DECODE_AVC_SLICE_PACKET_XE3_LPM_BASE_H__
