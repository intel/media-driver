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
//! \file     decode_hevc_mv_buffers.cpp
//! \brief    Defines MV buffers related logic for hevc decode
//!

#include "decode_hevc_basic_feature.h"
#include "decode_utils.h"
#include "decode_hevc_mv_buffers.h"
#include "codec_def_decode_hevc.h"

namespace decode
{

MOS_STATUS HevcMvBufferOpInf::Init(void* hwInterface, DecodeAllocator& allocator,
                                   HevcBasicFeature& basicFeature)
{
    DECODE_CHK_STATUS(BufferOpInf::Init(hwInterface, allocator, basicFeature));

    return MOS_STATUS_SUCCESS;
}

MOS_BUFFER* HevcMvBufferOpInf::Allocate()
{
    DECODE_FUNC_CALL();

    uint32_t mvtSize    = ((((m_basicFeature->m_width + 63) >> 6) * (((m_basicFeature->m_height + 15) >> 4)) + 1)&(-2));
    uint32_t mvtbSize   = ((((m_basicFeature->m_width + 31) >> 5) * (((m_basicFeature->m_height + 31) >> 5)) + 1)&(-2));
    uint32_t bufferSize = MOS_MAX(mvtSize, mvtbSize) * MHW_CACHELINE_SIZE;

    auto buffer = m_allocator->AllocateBuffer(bufferSize, "MvTemporalBuffer",
        resourceInternalReadWriteCache, notLockableVideoMem);

    return buffer;
}

MOS_STATUS HevcMvBufferOpInf::Resize(MOS_BUFFER* &buffer)
{
    DECODE_FUNC_CALL();

    if (buffer == nullptr)
    {
        DECODE_CHK_NULL(buffer = Allocate());
        return MOS_STATUS_SUCCESS;
    }

    uint32_t mvtSize    = ((((m_basicFeature->m_width + 63) >> 6) * (((m_basicFeature->m_height + 15) >> 4)) + 1)&(-2));
    uint32_t mvtbSize   = ((((m_basicFeature->m_width + 31) >> 5) * (((m_basicFeature->m_height + 31) >> 5)) + 1)&(-2));
    uint32_t bufferSize = MOS_MAX(mvtSize, mvtbSize) * MHW_CACHELINE_SIZE;

    auto status = m_allocator->Resize(buffer, bufferSize, notLockableVideoMem, false);
    return status;
}

void HevcMvBufferOpInf::Destroy(MOS_BUFFER* &buffer)
{
    DECODE_FUNC_CALL();

    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(buffer);
    }
}

}  // namespace decode
