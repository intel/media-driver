/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_vvc_mv_buffers.cpp
//! \brief    Defines MV buffers related logic for vvc decode
//!

#include "decode_vvc_basic_feature.h"
#include "decode_utils.h"
#include "decode_vvc_mv_buffers.h"
#include "codec_def_decode_vvc.h"

namespace decode
{

MOS_STATUS VvcMvBufferOpInf::Init(void* hwInterface, DecodeAllocator& allocator,
                                   VvcBasicFeature& basicFeature)
{
    DECODE_CHK_STATUS(BufferOpInf::Init(hwInterface, allocator, basicFeature));
    if (m_hwInterface != nullptr)
    {
        auto itf  = static_cast<CodechalHwInterfaceNext *>(m_hwInterface);
        m_vvcpItf = std::static_pointer_cast<mhw::vdbox::vvcp::Itf>(itf->GetVvcpInterfaceNext());
    }
    DECODE_CHK_NULL(m_vvcpItf);

    return MOS_STATUS_SUCCESS;
}

MOS_BUFFER* VvcMvBufferOpInf::Allocate()
{
    DECODE_FUNC_CALL();

    mhw::vdbox::vvcp::VvcpBufferSizePar vvcpBufSizeParam;
    MOS_ZeroMemory(&vvcpBufSizeParam, sizeof(vvcpBufSizeParam));
    vvcpBufSizeParam.m_picWidth  = m_basicFeature->m_width;
    vvcpBufSizeParam.m_picHeight = m_basicFeature->m_height;
    if (m_vvcpItf->GetVvcpBufSize(mhw::vdbox::vvcp::vcMvTemporalBuffer, &vvcpBufSizeParam) != MOS_STATUS_SUCCESS)
    {
        return nullptr;
    }

    return m_allocator->AllocateBuffer(
        vvcpBufSizeParam.m_bufferSize,
        "MvTemporalBuffer",
        resourceInternalReadWriteCache,
        notLockableVideoMem);
}

MOS_STATUS VvcMvBufferOpInf::Resize(MOS_BUFFER* &buffer)
{
    DECODE_FUNC_CALL();

    if (buffer == nullptr)
    {
        DECODE_CHK_NULL(buffer = Allocate());
        return MOS_STATUS_SUCCESS;
    }

    mhw::vdbox::vvcp::VvcpBufferSizePar vvcpBufSizeParam;
    MOS_ZeroMemory(&vvcpBufSizeParam, sizeof(vvcpBufSizeParam));
    vvcpBufSizeParam.m_picWidth  = m_basicFeature->m_width;
    vvcpBufSizeParam.m_picHeight = m_basicFeature->m_height;
    DECODE_CHK_STATUS(m_vvcpItf->GetVvcpBufSize(
        mhw::vdbox::vvcp::vcMvTemporalBuffer,
        &vvcpBufSizeParam));

    return m_allocator->Resize(buffer, vvcpBufSizeParam.m_bufferSize, notLockableVideoMem, false);
}

void VvcMvBufferOpInf::Destroy(MOS_BUFFER* &buffer)
{
    DECODE_FUNC_CALL();
    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(buffer);
    }
}

}  // namespace decode
