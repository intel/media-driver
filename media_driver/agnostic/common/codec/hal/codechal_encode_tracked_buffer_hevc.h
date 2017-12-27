/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_encode_tracked_buffer_hevc.h
//!
//! \brief    HEVC does not need to keep ref frame's PakObject
//!

#ifndef __CODECHAL_ENCODE_TRACKED_BUFFER_HEVC_H__
#define __CODECHAL_ENCODE_TRACKED_BUFFER_HEVC_H__

#include "codechal_encode_tracked_buffer.h"
#include "codechal_encode_hevc_base.h"

//!
//! Tracked buffer 
//!
class CodechalEncodeTrackedBufferHevc : public CodechalEncodeTrackedBuffer
{
public:
    //!
    //! \brief    Constructor
    //!            
    CodechalEncodeTrackedBufferHevc(CodechalEncoderState* encoder);

    ~CodechalEncodeTrackedBufferHevc()
    {}

private:
    CodechalEncodeTrackedBufferHevc(const CodechalEncodeTrackedBufferHevc&) = delete;
    CodechalEncodeTrackedBufferHevc& operator=(const CodechalEncodeTrackedBufferHevc&) = delete;

    void LookUpBufIndexMbCode() override;
    MOS_STATUS AllocateMvTemporalBuffer(uint8_t bufIndex) override;
    void ReleaseBufferOnResChange() override;

    CodechalEncodeHevcBase*         m_hevcState = nullptr;                      //!< HEVC encoder state
    uint8_t                         m_mbCodePenuIdx = 0;                        //!< 2nd-to-last MbCode buffer index
    uint8_t                         m_mbCodeAnteIdx = 0;                        //!< 3rd-to-last MbCode buffer index
};

#endif  // __CODECHAL_ENCODE_TRACKED_BUFFER_HEVC_H__
