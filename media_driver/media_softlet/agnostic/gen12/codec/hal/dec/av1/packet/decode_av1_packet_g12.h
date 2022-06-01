/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_av1_packet_g12.h
//! \brief    Defines the implementation of av1 decode packet of Gen12
//!

#ifndef __DECODE_AV1_PACKET_G12_H__
#define __DECODE_AV1_PACKET_G12_H__

#include "mhw_vdbox_g12_X.h"
#include "mhw_vdbox_avp_g12_X.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "mhw_mi_g12_X.h"
#include "mhw_render_g12_X.h"
#include "decode_av1_packet_g12_base.h"

namespace decode
{

class Av1DecodePktG12 : public Av1DecodePkt_G12_Base
{
public:
    Av1DecodePktG12(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface) :
        Av1DecodePkt_G12_Base(pipeline, task, hwInterface)
    {
        m_hwInterface = hwInterface;
    }

    virtual ~Av1DecodePktG12() {}

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
    MOS_STATUS PackTileLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS VdMemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS InitDummyWL(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer);

    MhwVdboxVdencInterface          *m_vdencInterface   = nullptr;
    MhwVdboxAvpInterface            *m_avpInterface     = nullptr;
    CodechalHwInterface             *m_hwInterface      = nullptr;
MEDIA_CLASS_DEFINE_END(decode__Av1DecodePktG12)
};

}

#endif
