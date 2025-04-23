/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_vp9_pipeline_xe2_hpm.cpp
//! \brief    Defines the interface for vp9 decode pipeline
//!
#include "decode_vp9_pipeline_xe2_hpm.h"
#include "decode_vp9_downsampling_packet_xe2_hpm.h"
#include "decode_vp9_packet_single_xe_lpm_plus_base.h"
#include "decode_vp9_packet_front_end_xe_lpm_plus_base.h"
#include "decode_vp9_packet_back_end_xe_lpm_plus_base.h"
#include "decode_vp9_picture_packet_xe_lpm_plus_base.h"
#include "decode_vp9_slice_packet_xe_lpm_plus_base.h"
#include "decode_vp9_mem_compression_xe2_hpm.h"

namespace decode
{
Vp9PipelineXe2_Hpm::Vp9PipelineXe2_Hpm(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Vp9PipelineXe_Lpm_Plus_Base(hwInterface, debugInterface)
{
}

MOS_STATUS Vp9PipelineXe2_Hpm::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(Vp9Pipeline::CreateSubPackets(subPacketManager, codecSettings));

#ifdef _DECODE_PROCESSING_SUPPORTED
    Vp9DownSamplingPktXe2_Hpm *downSamplingPkt = MOS_New(Vp9DownSamplingPktXe2_Hpm, this, m_hwInterface);
    DECODE_CHK_NULL(downSamplingPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, downSamplingSubPacketId), *downSamplingPkt));
#endif

    Vp9DecodePicPktXe_Lpm_Plus_Base *pictureDecodePkt = MOS_New(Vp9DecodePicPktXe_Lpm_Plus_Base, this, m_hwInterface);
    DECODE_CHK_NULL(pictureDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vp9PictureSubPacketId), *pictureDecodePkt));

    //VP9 slice packet, only 1 slice
    Vp9DecodeSlcPktXe_Lpm_Plus_Base *sliceDecodePkt = MOS_New(Vp9DecodeSlcPktXe_Lpm_Plus_Base, this, m_hwInterface);
    DECODE_CHK_NULL(sliceDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vp9SliceSubPacketId), *sliceDecodePkt));

    Vp9DecodeTilePktXe_Lpm_Plus_Base *tileDecodePkt = MOS_New(Vp9DecodeTilePktXe_Lpm_Plus_Base, this, m_hwInterface);
    DECODE_CHK_NULL(tileDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vp9TileSubPacketId), *tileDecodePkt));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineXe2_Hpm::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    DECODE_CHK_NULL(m_hwInterface);
    m_mmcState = MOS_New(Vp9DecodeMemCompXe2_Hpm, m_hwInterface);
    DECODE_CHK_NULL(m_mmcState);
    DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
#endif
    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
