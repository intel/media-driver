/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     encode_vp9_dynamic_scal_packet_xe_lpm_plus.h
//! \brief    Defines the interface to vp9 dynamic scaling (reference frame scaling) packet
//!

#ifndef __ENCODE_VP9_DYNAMIC_SCAL_PACKET_XE_LPMP_H__
#define __ENCODE_VP9_DYNAMIC_SCAL_PACKET_XE_LPMP_H__

#include "encode_vp9_vdenc_packet.h"
#include "encode_vp9_dynamic_scal_packet_xe_lpm_plus_base.h"

namespace encode
{

class Vp9DynamicScalPktXe_Lpm_Plus : public Vp9DynamicScalPktXe_Lpm_Plus_Base
{
public:
    //!
    //! \brief  Vp9DysRefFramePkt constructor
    //!
    Vp9DynamicScalPktXe_Lpm_Plus(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : Vp9DynamicScalPktXe_Lpm_Plus_Base(pipeline, task, hwInterface) {}

    //!
    //! \brief  Vp9DysRefFramePkt destructor
    //!
    virtual ~Vp9DynamicScalPktXe_Lpm_Plus(){};

    virtual MOS_STATUS Init() override;

MEDIA_CLASS_DEFINE_END(encode__Vp9DynamicScalPktXe_Lpm_Plus)
};

} // namespace encode

#endif  // !__ENCODE_VP9_DYNAMIC_SCAL_PACKET_XE_LPMP_H__