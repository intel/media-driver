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
//! \file      cm_state_buffer.h
//! \brief     Declaration of CmStateBuffer.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSTATEBUFFER_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSTATEBUFFER_H_

#include "cm_buffer_rt.h"
#include "cm_hal.h"

namespace CMRT_UMD
{
class CmStateBuffer: public CmBuffer_RT
{
public:
    static int32_t Create(uint32_t index,
                          uint32_t handle,
                          uint32_t size,
                          CmSurfaceManager *surfaceManager,
                          CM_STATE_BUFFER_TYPE stateBufferType,
                          CmStateBuffer *&surface);

    CM_RT_API CM_ENUM_CLASS_TYPE Type() const
    { return CM_ENUM_CLASS_TYPE_CM_STATE_BUFFER; };

protected:
    CmStateBuffer(uint32_t handle,
                  uint32_t size,
                  CmSurfaceManager *surfaceManager,
                  CM_STATE_BUFFER_TYPE stateBufferType);

    ~CmStateBuffer();

    CM_STATE_BUFFER_TYPE m_stateBufferType;
};
}

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSTATEBUFFER_H_
