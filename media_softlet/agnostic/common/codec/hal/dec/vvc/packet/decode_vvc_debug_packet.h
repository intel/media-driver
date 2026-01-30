/*
* Copyright (c) 2026, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice (including the next
* paragraph) shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/
//!
//! \file     decode_vvc_debug_packet.h
//! \brief    Defines the implementation of VVC decode debug packet
//!

#ifndef __DECODE_VVC_DEBUG_PACKET_H__
#define __DECODE_VVC_DEBUG_PACKET_H__

#include "decode_sub_packet.h"
#include "media_cmd_packet.h"
#include "mhw_mi_itf.h"

namespace decode
{

#if (_DEBUG || _RELEASE_INTERNAL)
class VvcDecodeDebugPkt : public DecodeSubPacket
{
public:
    //!
    //! \brief  VVC decode debug sub packet constructor
    //!
    VvcDecodeDebugPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief  VVC decode debug sub packet destructor
    //!
    virtual ~VvcDecodeDebugPkt() = default;

    //!
    //! \brief  Initialize the debug sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Prepare the parameters for debug sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    virtual MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

    //!
    //! \brief  Destroy the debug sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy();

    //!
    //! \brief  Execute debug sub packet
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //! \param  [in] statusReport
    //!         Media status report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, MediaStatusReport *statusReport);

    //!
    //! \brief  Complete debug operations after decode
    //! \param  [in] mfxStatus
    //!         Pointer to DecodeStatusMfx structure containing command counter value
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Completed(void *mfxStatus);

    //!
    //! \brief  Add command counter override commands
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //! \param  [in] statusReport
    //!         Media status report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddCommandCounterCmds(MOS_COMMAND_BUFFER& cmdBuffer, MediaStatusReport *statusReport);

protected:
    std::shared_ptr<mhw::mi::Itf> m_miItf = nullptr;

MEDIA_CLASS_DEFINE_END(decode__VvcDecodeDebugPkt)
};
#endif

}

#endif