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
//! \file      cm_state_buffer.cpp
//! \brief     Implementation of CmStateBuffer.
//!

#include "cm_state_buffer.h"
#include "cm_debug.h"

namespace CMRT_UMD
{
CmStateBuffer::CmStateBuffer(uint32_t handle,
                             uint32_t size,
                             CmSurfaceManager *surfaceManager,
                             CM_STATE_BUFFER_TYPE stateBufferType)
    : CmBuffer_RT(handle, size, true, surfaceManager, 0, false, nullptr, false,
                  0, 0),
      m_stateBufferType(stateBufferType) {}

CMRT_UMD::CmStateBuffer::~CmStateBuffer() {}

int32_t CmStateBuffer::Create(uint32_t index,
                              uint32_t handle,
                              uint32_t size,
                              CmSurfaceManager *surfaceManager,
                              CM_STATE_BUFFER_TYPE stateBufferType,
                              CmStateBuffer *&surface)
{
    int32_t result = CM_SUCCESS;

    surface = new (std::nothrow)
        CmStateBuffer(handle, size, surfaceManager, stateBufferType);
    if (surface)
    {
        result = surface->Initialize(index);
        if (result != CM_SUCCESS)
        {
            CmSurface *baseSurface = surface;
            CmSurface::Destroy(baseSurface);
        }
    }
    else
    {
        CM_ASSERTMESSAGE(
            "Error: Failed to create CmStateBuffer due to out of system "
            "memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}
}
