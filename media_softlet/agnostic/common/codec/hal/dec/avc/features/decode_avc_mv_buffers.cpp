/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_avc_mv_buffers.cpp
//! \brief    Defines MV buffers related logic for avc decode
//!

#include "decode_avc_basic_feature.h"
#include "decode_utils.h"
#include "decode_avc_mv_buffers.h"
#include "codec_def_decode_avc.h"

namespace decode
{

MOS_STATUS AvcMvBufferOpInf::Init(void* hwInterface, DecodeAllocator& allocator,
                                   AvcBasicFeature& basicFeature)
{
    DECODE_CHK_STATUS(BufferOpInf::Init(hwInterface, allocator, basicFeature));
    return MOS_STATUS_SUCCESS;
}

MOS_BUFFER* AvcMvBufferOpInf::Allocate()
{
    DECODE_FUNC_CALL();

    m_picWidthInMB =  (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_basicFeature->m_width);
    m_picHeightInMB = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_basicFeature->m_height);

    uint32_t avcDmvBufferSize = 64 * m_picWidthInMB * (uint32_t)(MOS_ALIGN_CEIL(m_picHeightInMB, 2));
    return m_allocator->AllocateBuffer(avcDmvBufferSize, "AvcMvBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
}

MOS_STATUS AvcMvBufferOpInf::Resize(MOS_BUFFER* &buffer)
{
    DECODE_FUNC_CALL();

    if (buffer == nullptr)
    {
        DECODE_CHK_NULL(buffer = Allocate());
        return MOS_STATUS_SUCCESS;
    }

    m_picWidthInMB  = MOS_MAX(m_picWidthInMB, m_basicFeature->m_avcPicParams->pic_width_in_mbs_minus1 + 1);
    m_picHeightInMB = MOS_MAX(m_picHeightInMB, m_basicFeature->m_avcPicParams->pic_height_in_mbs_minus1 + 1);

    uint32_t newDmvBufferSize = 64 * m_picWidthInMB * (uint32_t)(MOS_ALIGN_CEIL(m_picHeightInMB, 2));
    return m_allocator->Resize(buffer, newDmvBufferSize, notLockableVideoMem, false);
}

void AvcMvBufferOpInf::Destroy(MOS_BUFFER* &buffer)
{
    DECODE_FUNC_CALL();

    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(buffer);
    }
}

}  // namespace decode
