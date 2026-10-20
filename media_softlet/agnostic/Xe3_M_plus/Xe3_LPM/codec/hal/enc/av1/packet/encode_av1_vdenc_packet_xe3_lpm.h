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
//! \file     encode_av1_vdenc_packet_xe3_lpm.h
//! \brief    Defines the interface to av1 vdenc encode packet Xe3_LPM
//!

#ifndef _ENCODE_AV1_VDENC_PACKET_XE3_LPM_H__
#define _ENCODE_AV1_VDENC_PACKET_XE3_LPM_H__

#include "encode_av1_vdenc_packet_xe3_lpm_base.h"
#include "mhw_vdbox_aqm_impl_xe3_lpm.h"

namespace encode
{
class Av1VdencPktXe3_Lpm : public Av1VdencPktXe3_Lpm_Base
{
public:
    Av1VdencPktXe3_Lpm(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : Av1VdencPktXe3_Lpm_Base(pipeline, task, hwInterface)
    {
        m_aqmItf = std::make_shared<mhw::vdbox::aqm::xe3_lpm::Impl>(m_osInterface);
    }

    virtual ~Av1VdencPktXe3_Lpm(){};

protected:

MEDIA_CLASS_DEFINE_END(encode__Av1VdencPktXe3_Lpm)
};
}  // namespace encode
#endif  //_ENCODE_AV1_VDENC_PACKET_XE3_LPM_H__
