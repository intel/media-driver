/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_vp9_packet_front_end_xe_m_base.h
//! \brief    Defines the implementation of vp9 front end decode packet
//!

#ifndef __DECODE_VP9_PACKET_FRONT_END_XE_M_BASE_H__
#define __DECODE_VP9_PACKET_FRONT_END_XE_M_BASE_H__

#include "decode_vp9_packet_xe_m_base.h"

namespace decode
{

class Vp9DecodeFrontEndPktXe_M_Base : public Vp9DecodePktXe_M_Base
{
public:
    Vp9DecodeFrontEndPktXe_M_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface)
        : Vp9DecodePktXe_M_Base(pipeline, task, hwInterface)
    {
    }
    virtual ~Vp9DecodeFrontEndPktXe_M_Base();

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

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
    MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "VP9_DECODE_FRONT_END_PASS" + std::to_string(static_cast<uint32_t>(m_vp9Pipeline->GetCurrentPass()));
    }

protected:
    //!
    //! \brief  Calculate Command Buffer Size
    //! \param  [out] commandBufferSize
    //!         requested size
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateCommandBufferSize(uint32_t &commandBufferSize);

    //!
    //! \brief  Calculate Patch List Size
    //! \param  [out] requestedPatchListSize
    //!         requested size
    //! \return uint32_t
    //!         Patchlist size calculated
    //!
    virtual MOS_STATUS CalculatePatchListSize(uint32_t &requestedPatchListSize);

    //!
    //! \brief  Check if prolog required
    //!
    //! \return bool
    //!         True if prolog required
    //!
    bool IsPrologRequired();

    Vp9DecodePicPktXe_M_Base *       m_picturePkt = nullptr;
    Vp9DecodeSlcPktXe_M_Base *       m_slicePkt   = nullptr;

MEDIA_CLASS_DEFINE_END(decode__Vp9DecodeFrontEndPktXe_M_Base)
};

}  // namespace decode
#endif
