/*
* Copyright (c) 2022-2025, Intel Corporation
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
//! \file     decode_avc_packet_xe3_lpm_base.h
//! \brief    Defines the implementation of avc decode packet of Xe3_LPM+
//!

#ifndef __DECODE_AVC_PACKET_XE3_LPM_BASE_H__
#define __DECODE_AVC_PACKET_XE3_LPM_BASE_H__

#include "mhw_vdbox_xe_lpm_plus_base.h"
#include "codec_hw_xe3_lpm_base.h"
#include "decode_avc_packet.h"
#include "decode_avc_aqm_packet_xe3_lpm_base.h"

namespace decode
{

class AvcDecodePktXe3_Lpm_Base : public AvcDecodePkt
{
public:
    AvcDecodePktXe3_Lpm_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : 
        AvcDecodePkt(pipeline, task, hwInterface)
    {
        m_hwInterface = dynamic_cast<CodechalHwInterfaceXe3_Lpm_Base*>(hwInterface);
        m_vdencItf    = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(hwInterface->GetVdencInterfaceNext());
    }

    virtual ~AvcDecodePktXe3_Lpm_Base(){};

    virtual MOS_STATUS Init() override;
    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override;

#ifdef _DECODE_PROCESSING_SUPPORTED
    AvcDecodeAqmPktXe3LpmBase *m_aqmPkt = nullptr;
#endif

protected:
    MOS_STATUS PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS VdPipelineFlushVdaqm(MOS_COMMAND_BUFFER &cmdBuffer);

    CodechalHwInterfaceXe3_Lpm_Base* m_hwInterface = nullptr;
    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;

MEDIA_CLASS_DEFINE_END(decode__AvcDecodePktXe3_Lpm_Base)
};

}

#endif  //!__DECODE_AVC_PACKET_XE3_LPM_BASE_H__
