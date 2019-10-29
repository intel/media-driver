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
//! \file      cm_scratch_space.h
//! \brief     Contains Class CmScratchSpace definitions 
//!

#pragma once

#include "cm_hal.h"

namespace CMRT_UMD
{
    class CmDeviceRT;
    class CmBuffer;
};
class CmKernelEx;

class CmScratchSpace
{
public:
    CmScratchSpace();
    ~CmScratchSpace();

    MOS_STATUS Initialize(CMRT_UMD::CmDeviceRT *device);
    MOS_STATUS Allocate(CmKernelEx **kernels, uint32_t count);
    void Submit(uint32_t trackerIndex, uint32_t tracker);

    inline MOS_RESOURCE *GetResource() {return m_resource; }
    inline uint32_t GetSize() {return m_scratchSize; }

protected:
    CMRT_UMD::CmDeviceRT *m_device;
    CM_HAL_STATE *m_cmhal;
    bool m_isSeparated;
    MOS_RESOURCE *m_resource;
    uint32_t m_scratchSize;
    CMRT_UMD::CmBuffer *m_buffer; // wrapper of resource for delay destroy
    
};
