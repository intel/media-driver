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
//! \file     decode_av1_temporal_buffers.h
//! \brief    Defines temporal buffers related logic for av1 decode
//!
#ifndef __DECODE_AV1_TEMPORAL_BUFFERS_H__
#define __DECODE_AV1_TEMPORAL_BUFFERS_H__

#include "codec_def_decode_av1.h"
#include "decode_allocator.h"
#include "mhw_vdbox_avp_itf.h"
#include "decode_reference_associated_buffer.h"

using AvpBufferSizePar = mhw::vdbox::avp::AvpBufferSizePar;
using AvpBufferType = mhw::vdbox::avp::AvpBufferType;

namespace decode
{
    class Av1BasicFeature;

    class Av1TempBufferOpInf : public BufferOpInf<Av1RefAssociatedBufs, Av1BasicFeature>
    {
    public:
        ~Av1TempBufferOpInf() {};
        virtual MOS_STATUS Init(void* hwInterface, DecodeAllocator& allocator,
                        Av1BasicFeature& basicFeature);
        virtual Av1RefAssociatedBufs *Allocate();
        virtual MOS_STATUS Resize(Av1RefAssociatedBufs* &buffer);
        virtual MOS_STATUS Deactive(Av1RefAssociatedBufs* &buffer);
        virtual bool IsAvailable(Av1RefAssociatedBufs* &buffer);
        virtual void Destroy(Av1RefAssociatedBufs* &buffer);
    protected:
        void                  SetAvpBufSizeParam(AvpBufferSizePar &params, int32_t mibSizeLog2);
        void                  RecordSegIdBufInfo(Av1RefAssociatedBufs *buffer);
        void                  RecordCdfTableBufInfo(Av1RefAssociatedBufs *buffer);
        inline Av1SharedBuf  *RefSharedBuffer(Av1SharedBuf *sharedBuf);
        inline Av1SharedBuf  *DeRefSharedBuffer(Av1SharedBuf *sharedBuf);
        std::shared_ptr<mhw::vdbox::avp::Itf> m_avpItf = nullptr;
        int32_t               widthInSb                = 0;
        int32_t               heightInSb               = 0;

    MEDIA_CLASS_DEFINE_END(decode__Av1TempBufferOpInf)
    };

}  // namespace decode

#endif  // !__DECODE_AV1_TEMPORAL_BUFFERS_H__
