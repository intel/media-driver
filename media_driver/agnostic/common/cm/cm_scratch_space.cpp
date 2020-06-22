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
//! \file      cm_scratch_space.cpp
//! \brief     Contains Class CmScratchSpace definitions 
//!
#include "cm_scratch_space.h"
#include "cm_device_rt.h"
#include "cm_buffer_rt.h"
#include "cm_kernel_ex.h"

using namespace CMRT_UMD;

CmScratchSpace::CmScratchSpace():
    m_device(nullptr),
    m_cmhal(nullptr),
    m_isSeparated(false),
    m_resource(nullptr),
    m_scratchSize(0),
    m_buffer(nullptr)
{
}

CmScratchSpace::~CmScratchSpace()
{
    if (m_device && m_buffer)
    {
        m_device->DestroySurface(m_buffer);
    }
}

MOS_STATUS CmScratchSpace::Initialize(CmDeviceRT *device)
{
    CM_CHK_NULL_RETURN_MOSERROR(device);
    m_device = device;
    CM_CHK_NULL_RETURN_MOSERROR(device->GetAccelData());
    m_cmhal = ((PCM_CONTEXT_DATA)device->GetAccelData())->cmHalState;
    CM_CHK_NULL_RETURN_MOSERROR(m_cmhal);
    if (m_cmhal->cmHalInterface->IsSeparateScratch())
    {
        m_isSeparated = true;
        return MOS_STATUS_SUCCESS;
    }
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS CmScratchSpace::Allocate(CmKernelEx **kernels, uint32_t count)
{
    if (!m_isSeparated)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
    uint32_t maxSpillSize = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        CmKernelEx *kernel = kernels[i];
        // get the spill size
        maxSpillSize = MOS_MAX(maxSpillSize, kernel->GetSpillMemUsed());
    }

    if (maxSpillSize == 0)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    uint32_t perThreadScratchSpace = 64;
    for (perThreadScratchSpace; perThreadScratchSpace < maxSpillSize; perThreadScratchSpace <<= 1);

    // get max thread number
    MEDIA_SYSTEM_INFO *gtSystemInfo = m_cmhal->osInterface->pfnGetGtSystemInfo(m_cmhal->osInterface);
    uint32_t numHWThreadsPerEU = gtSystemInfo->ThreadCount / gtSystemInfo->EUCount;
    uint32_t maxHWThreads = gtSystemInfo->MaxEuPerSubSlice * numHWThreadsPerEU * gtSystemInfo->MaxSubSlicesSupported;
    m_scratchSize = maxHWThreads * perThreadScratchSpace;

    // create the scratch space by CmBuffer
    m_device->CreateBuffer(m_scratchSize, m_buffer);
    CM_CHK_NULL_RETURN_MOSERROR(m_buffer);
    CmBuffer_RT *bufferRT = static_cast<CmBuffer_RT *>(m_buffer);
    uint32_t handle = 0;
    bufferRT->GetHandle(handle);
    m_resource = &m_cmhal->bufferTable[handle].osResource;

    return MOS_STATUS_SUCCESS;
}

void CmScratchSpace::Submit(uint32_t trackerIndex, uint32_t tracker)
{
    if (m_buffer == nullptr || m_scratchSize == 0)
    {
        return;
    }
    CmBuffer_RT *bufferRT = static_cast<CmBuffer_RT *>(m_buffer);
    bufferRT->SetFastTracker(trackerIndex, tracker);
}
