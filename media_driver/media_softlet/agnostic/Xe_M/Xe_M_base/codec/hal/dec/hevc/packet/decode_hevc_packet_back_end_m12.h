/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_hevc_packet_back_end_m12.h
//! \brief    Defines the implementation of hevc back end decode packet of M12
//!

#ifndef __DECODE_HEVC_PACKET_BACK_END_M12_H__
#define __DECODE_HEVC_PACKET_BACK_END_M12_H__

#include "decode_hevc_packet_back_end_xe_m_base.h"
#include "decode_hevc_tile_packet_m12.h"
#include "codechal_hw_g12_X.h"

namespace decode
{

class HevcDecodeBackEndPktM12 : public HevcDecodeBackEndPktXe_M_Base
{
public:
    HevcDecodeBackEndPktM12(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface)
        : HevcDecodeBackEndPktXe_M_Base(pipeline, task, hwInterface)
    {
        m_hwInterface = dynamic_cast<CodechalHwInterfaceG12*>(hwInterface);
    }

    virtual ~HevcDecodeBackEndPktM12() {}

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
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

protected:
    MOS_STATUS PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS VdMemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS VdPipelineFlush(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS VdScalabPipeUnLock(MOS_COMMAND_BUFFER &cmdBuffer);

    CodechalHwInterfaceG12* m_hwInterface = nullptr;
    HevcDecodeTilePktM12*   m_tilePkt     = nullptr;
MEDIA_CLASS_DEFINE_END(decode__HevcDecodeBackEndPktM12)
};

}

#endif
