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
//! \file     decode_vp9_packet_single_xe_lpm_plus_base.h
//! \brief    Defines the implementation of vp9 single decode packet of Xe_LPM_plus+
//!

#ifndef __DECODE_VP9_PACKET_SINGLE_XE_LPM_PLUS_BASE_H__
#define __DECODE_VP9_PACKET_SINGLE_XE_LPM_PLUS_BASE_H__

#include "decode_vp9_packet_single.h"
#include "codec_hw_xe_lpm_plus_base.h"

namespace decode
{

class Vp9DecodeSinglePktXe_Lpm_Plus_Base : public Vp9DecodeSinglePkt
{
public:
    Vp9DecodeSinglePktXe_Lpm_Plus_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
        Vp9DecodeSinglePkt(pipeline, task, hwInterface)
    {
        m_hwInterface = dynamic_cast<CodechalHwInterfaceXe_Lpm_Plus_Base *>(hwInterface);
        DECODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
        m_vdencItf    = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
    }

    virtual ~Vp9DecodeSinglePktXe_Lpm_Plus_Base() {}

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override;

protected:
    MOS_STATUS PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS VdMemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS VdPipelineFlush(MOS_COMMAND_BUFFER &cmdBuffer);

    CodechalHwInterfaceXe_Lpm_Plus_Base *m_hwInterface = nullptr;

MEDIA_CLASS_DEFINE_END(decode__Vp9DecodeSinglePktXe_Lpm_Plus_Base)
};

}

#endif
