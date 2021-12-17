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
#include "codechal_utilities.h"
#include "decode_hevc_mv_buffers.h"
#include "codec_def_decode_hevc.h"

namespace decode
{

MOS_STATUS HevcMvBufferOpInf::Init(CodechalHwInterface& hwInterface, DecodeAllocator& allocator,
                                   HevcBasicFeature& basicFeature)
{
    DECODE_CHK_STATUS(BufferOpInf::Init(hwInterface, allocator, basicFeature));
    m_hcpInterface = m_hwInterface->GetHcpInterface();
    DECODE_CHK_NULL(m_hcpInterface);
    return MOS_STATUS_SUCCESS;
}

MOS_BUFFER* HevcMvBufferOpInf::Allocate()
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.dwPicWidth  = m_basicFeature->m_width;
    hcpBufSizeParam.dwPicHeight = m_basicFeature->m_height;
    if (m_hcpInterface->GetHevcBufferSize(MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL, 
                                        &hcpBufSizeParam) != MOS_STATUS_SUCCESS)
    {
        return nullptr;
    }

    return m_allocator->AllocateBuffer(hcpBufSizeParam.dwBufferSize, "MvTemporalBuffer",
        resourceInternalReadWriteCache, notLockableVideoMem);
}

MOS_STATUS HevcMvBufferOpInf::Resize(MOS_BUFFER* &buffer)
{
    DECODE_FUNC_CALL();

    if (buffer == nullptr)
    {
        DECODE_CHK_NULL(buffer = Allocate());
        return MOS_STATUS_SUCCESS;
    }

    MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.dwPicWidth  = m_basicFeature->m_width;
    hcpBufSizeParam.dwPicHeight = m_basicFeature->m_height;
    DECODE_CHK_STATUS(m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL,
        &hcpBufSizeParam));

    return m_allocator->Resize(buffer, hcpBufSizeParam.dwBufferSize, notLockableVideoMem, false);
}

void HevcMvBufferOpInf::Destroy(MOS_BUFFER* &buffer)
{
    DECODE_FUNC_CALL();
    m_allocator->Destroy(buffer);
}

}  // namespace decode
