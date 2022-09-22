/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_hevc_mv_buffers.h
//! \brief    Defines MV buffer related logic for hevc decode
//!
#ifndef __DECODE_HEVC_MV_BUFFERS_H__
#define __DECODE_HEVC_MV_BUFFERS_H__

#include "codec_def_decode_hevc.h"
#include "decode_allocator.h"
#include "mhw_vdbox_hcp_itf.h"
#include "decode_reference_associated_buffer.h"

namespace decode
{
class HevcBasicFeature;

class HevcMvBufferOpInf : public BufferOpInf<MOS_BUFFER, HevcBasicFeature>
{
public:
    virtual MOS_STATUS Init(void* hwInterface, DecodeAllocator& allocator,
                    HevcBasicFeature& basicFeature);
    virtual MOS_BUFFER *Allocate();
    virtual MOS_STATUS Resize(MOS_BUFFER* &buffer);
    virtual void Destroy(MOS_BUFFER* &buffer);

MEDIA_CLASS_DEFINE_END(decode__HevcMvBufferOpInf)
};

}  // namespace decode

#endif  // !__DECODE_HEVC_MV_BUFFERS_H__
