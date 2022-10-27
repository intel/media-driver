/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_hevc_slice_packet_xe_lpm_plus_base.h
//! \brief    Defines the implementation of hevc decode slice packet for Xe_LPM_plus+
//!

#ifndef __DECODE_HEVC_SLICE_PACKET_XE_LPM_PLUS_BASE_H__
#define __DECODE_HEVC_SLICE_PACKET_XE_LPM_PLUS_BASE_H__

#include "media_cmd_packet.h"
#include "decode_hevc_pipeline.h"
#include "decode_utils.h"
#include "decode_hevc_basic_feature.h"
#include "decode_hevc_slice_packet.h"

namespace decode
{

class HevcDecodeSlcPktXe_Lpm_Plus_Base : public HevcDecodeSlcPkt
{
public:
    HevcDecodeSlcPktXe_Lpm_Plus_Base(HevcPipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
        : HevcDecodeSlcPkt(pipeline, hwInterface)
    {
    }
    virtual ~HevcDecodeSlcPktXe_Lpm_Plus_Base();

    MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx) override;

protected:
    MOS_STATUS Set_HCP_SLICE_STATE(uint32_t sliceIdx, uint32_t subTileIdx);
    MOS_STATUS AddCmd_HCP_SLICE_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx);
    MOS_STATUS AddCmd_HCP_REF_IDX_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx);

MEDIA_CLASS_DEFINE_END(decode__HevcDecodeSlcPktXe_Lpm_Plus_Base)
};

}  // namespace decode
#endif
