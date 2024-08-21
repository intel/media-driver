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
//! \file     decode_avc_pipeline_xe2_hpm.cpp
//! \brief    Defines the interface for avc decode pipeline
//!
#include "decode_avc_pipeline_xe2_hpm.h"
#include "decode_avc_downsampling_packet_xe2_hpm.h"
#include "decode_avc_packet_xe_lpm_plus_base.h"
#include "decode_avc_slice_packet_xe_lpm_plus_base.h"
#include "decode_avc_picture_packet_xe_lpm_plus_base.h"
#include "decode_mem_compression_xe2_hpm.h"

namespace decode
{

AvcPipelineXe2_Hpm::AvcPipelineXe2_Hpm(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : AvcPipelineXe_Lpm_Plus_Base(hwInterface, debugInterface)
{
}

MOS_STATUS AvcPipelineXe2_Hpm::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(AvcPipeline::CreateSubPackets(subPacketManager, codecSettings));

#ifdef _DECODE_PROCESSING_SUPPORTED
    AvcDownSamplingPktXe2_Hpm *downSamplingPkt = MOS_New(AvcDownSamplingPktXe2_Hpm, this, m_hwInterface);
    DECODE_CHK_NULL(downSamplingPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, downSamplingSubPacketId), *downSamplingPkt));
#endif

    AvcDecodePicPktXe_Lpm_Plus_Base *pictureDecodePkt = MOS_New(AvcDecodePicPktXe_Lpm_Plus_Base, this, m_hwInterface);
    DECODE_CHK_NULL(pictureDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, avcPictureSubPacketId), *pictureDecodePkt));

    AvcDecodeSlcPktXe_Lpm_Plus_Base *sliceDecodePkt = MOS_New(AvcDecodeSlcPktXe_Lpm_Plus_Base, this, m_hwInterface);
    DECODE_CHK_NULL(sliceDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, avcSliceSubPacketId), *sliceDecodePkt));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipelineXe2_Hpm::InitMmcState()
{
    DECODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
    DECODE_CHK_NULL(m_hwInterface);
    m_mmcState = MOS_New(DecodeMemCompXe2_Hpm, m_hwInterface);
    DECODE_CHK_NULL(m_mmcState);

    DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
#endif
    return MOS_STATUS_SUCCESS;
}

}
