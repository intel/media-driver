/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     encode_vp9_vdenc_packet_xe_lpm_plus.h
//! \brief    Defines the interface to adapt to vp9 vdenc encode CMD packet
//!

#ifndef __ENCODE_VP9_VDENC_PACKET_XE_LPM_PLUS_H__
#define __ENCODE_VP9_VDENC_PACKET_XE_LPM_PLUS_H__

#include "encode_vp9_vdenc_packet_xe_lpm_plus_base.h"

namespace encode
{
class Vp9VdencPktXe_Lpm_Plus : public Vp9VdencPktXe_Lpm_Plus_Base
{
public:
    //!
    //! \brief  Vp9VdencPktXe_Lpm_Plus constructor
    //!
    Vp9VdencPktXe_Lpm_Plus(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : Vp9VdencPktXe_Lpm_Plus_Base(pipeline, task, hwInterface) {}

    //!
    //! \brief  Vp9VdencPktXe_Lpm_Plus destructor
    //!
    virtual ~Vp9VdencPktXe_Lpm_Plus() {}

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init() override;

MEDIA_CLASS_DEFINE_END(encode__Vp9VdencPktXe_Lpm_Plus)
};
}  // namespace encode

#endif  // !__ENCODE_VP9_VDENC_PACKET_XE_LPM_PLUS_H__