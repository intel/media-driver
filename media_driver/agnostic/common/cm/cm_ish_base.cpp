/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_ish_base.cpp
//! \brief     Contains Class CmISHBase  definitions
//!

#include "cm_ish.h"
#include "cm_kernel_ex.h"

using namespace CMRT_UMD;

CmISHBase::CmISHBase():
    m_osInterface(nullptr),
    m_resource(nullptr),
    m_lockedData(nullptr),
    m_size(0),
    m_offset(0),
    m_trackerProducer(nullptr),
    m_lastTrackerToken(nullptr),
    m_addedKernelCount(0),
    m_isSipKernelLoaded(false),
    m_sipKernel(nullptr),
    m_sipKernelSize(0),
    m_sipKernelOffset(0)
{

}

CmISHBase::~CmISHBase()
{
    while (m_destroyedTrackers.size())
    {
        MOS_RESOURCE *res = m_destroyedResources.back();
        m_osInterface->pfnFreeResourceWithFlag(m_osInterface, res, SURFACE_FLAG_ASSUME_NOT_IN_USE);

        FrameTrackerToken *trackerToken = m_destroyedTrackers.back();

        MOS_FreeMemory(res);
        MOS_Delete(trackerToken);
        m_destroyedResources.pop_back();
        m_destroyedTrackers.pop_back();
    }
    // delete current resource
    if (m_resource)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, m_resource);
        m_osInterface->pfnFreeResourceWithFlag(m_osInterface, m_resource, SURFACE_FLAG_ASSUME_NOT_IN_USE);
        MOS_FreeMemory(m_resource);
    }

    if (m_lastTrackerToken)
    {
        MOS_Delete(m_lastTrackerToken);
    }

    if (m_sipKernel)
    {
        MOS_FreeMemory(m_sipKernel);
    }
}

MOS_STATUS CmISHBase::Initialize(CM_HAL_STATE *cmhal, FrameTrackerProducer *trackerProducer)
{
    m_osInterface = cmhal->osInterface;
    m_trackerProducer = trackerProducer;
    ExpandHeapSize(m_initSize);

    if (!cmhal->midThreadPreemptionDisabled)
    {
        CM_CHK_MOSSTATUS_RETURN(CreateSipKernel(cmhal)); // generate sip kernel in m_sipKernel
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmISHBase::LoadKernels(CmKernelEx **kernels, int count)
{
    uint32_t blockSizes[CM_MAX_KERNELS_PER_TASK];
    uint32_t blockIdx = 0;
    uint32_t totalSize = 0;
    for (int i = 0; i < count; i++)
    {
        CmKernelEx *kernel = kernels[i];
        // check if the kernel is in ish
        CmISHBase *rIsh = nullptr;
        int rIdx = -1;
        kernel->GetRecordedInfo(&rIsh, &rIdx);
        if (rIsh == this && rIdx >= 0 && rIdx < (int)m_addedKernelCount && m_addedKernels[rIdx] == kernel)
        {
            //printf("HFDebug: reUse kernel %p in ish, name %s\n", kernel, kernel->GetName());
            continue;
        }
        auto ite = m_addedHashValues.find(kernel->GetHashValue());
        if (ite != m_addedHashValues.end())
        {
            continue;
        }
        //printf("HFDebug: add a new kernel %p in ish, name %s\n", kernel, kernel->GetName());
        uint8_t *nkernel = nullptr;
        uint32_t size = 0;
        kernel->GetNativeKernel(&nkernel, &size);
        uint32_t temp = MOS_ALIGN_CEIL(size + m_kernelPadding, m_kernelAlign);
        blockSizes[blockIdx ++] = temp;
        totalSize += temp;
    }

    uint32_t neededSize = totalSize;
    if (!m_isSipKernelLoaded && m_sipKernelSize)
    {
        neededSize += MOS_ALIGN_CEIL(m_sipKernelSize + m_kernelPadding, m_kernelAlign);
    }
    if (m_offset + neededSize > m_size)
    {
        ExpandHeapSize(neededSize);
    }

    if (!m_isSipKernelLoaded && m_sipKernelSize)
    {
        uint32_t temp = MOS_ALIGN_CEIL(m_sipKernelSize + m_kernelPadding, m_kernelAlign);

        // Copy Cm Kernel Binary
        MOS_SecureMemcpy(m_lockedData + m_offset, m_size - m_offset, m_sipKernel, m_sipKernelSize);

        // Padding bytes dummy instructions after kernel binary to resolve page fault issue
        MOS_ZeroMemory(m_lockedData + m_offset + m_sipKernelSize, temp - m_sipKernelSize);
        m_sipKernelOffset = m_offset;
        m_offset += temp;

        m_isSipKernelLoaded = true;
    }

    for (int i = 0; i < count; i++)
    {
        CmKernelEx *kernel = kernels[i];
        // check if the kernel is in ish
        CmISHBase *rIsh = nullptr;
        int rIdx = -1;
        kernel->GetRecordedInfo(&rIsh, &rIdx);
        if (rIsh == this && rIdx >= 0 && rIdx < (int)m_addedKernelCount && m_addedKernels[rIdx] == kernel)
        {
            continue;
        }
        auto ite = m_addedHashValues.find(kernel->GetHashValue());
        if (ite != m_addedHashValues.end())
        {
            uint32_t offset = ite->second;
            kernel->Recorded(this, -1, offset);
            continue;
        }
        // copy kernel binary
        uint8_t *nkernel = nullptr;
        uint32_t size = 0;
        kernel->GetNativeKernel(&nkernel, &size);
        uint32_t temp = MOS_ALIGN_CEIL(size + m_kernelPadding, m_kernelAlign);

        // Copy Cm Kernel Binary
        MOS_SecureMemcpy(m_lockedData + m_offset, m_size - m_offset, nkernel, size);

        // Padding bytes dummy instructions after kernel binary to resolve page fault issue
        MOS_ZeroMemory(m_lockedData + m_offset + size, temp - size);

        // record
        m_addedKernels.push_back(kernel);
        kernel->Recorded(this, (int)m_addedKernelCount, m_offset);
        m_addedHashValues[kernel->GetHashValue()] = m_offset;
        m_addedKernelCount ++;
        m_offset += temp;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmISHBase::ExpandHeapSize(uint32_t extraSize)
{
    uint32_t addSize = MOS_ALIGN_CEIL(extraSize, m_expandStep);
    // destroy the old resource
    if (m_resource)
    {
        m_destroyedResources.push_front(m_resource);
        m_destroyedTrackers.push_front(m_lastTrackerToken);
    }

    // allocate new resource
    m_resource = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE));
    CM_CHK_NULL_RETURN_MOSERROR(m_resource);
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(allocParams));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format = Format_Buffer;
    allocParams.dwBytes = m_size + addSize;
    allocParams.pBufName = "ISHeap";

    CM_CHK_MOSSTATUS_RETURN(m_osInterface->pfnAllocateResource(m_osInterface, &allocParams,
                                                               m_resource));
    CM_CHK_MOSSTATUS_RETURN(m_osInterface->pfnRegisterResource(m_osInterface, m_resource,
                                                               true, true));
    CM_CHK_MOSSTATUS_RETURN(m_osInterface->pfnSkipResourceSync(m_resource));

    m_size += addSize;
    m_offset = 0;

    MOS_LOCK_PARAMS lockParams;
    MOS_ZeroMemory(&lockParams, sizeof(lockParams));
    lockParams.WriteOnly = 1;
    lockParams.NoOverWrite = 1;
    lockParams.Uncached = 1;
    m_lockedData = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, m_resource, &lockParams);

    // one new tracker token with one new resource
    m_lastTrackerToken = MOS_New(FrameTrackerToken);
    m_lastTrackerToken->SetProducer(m_trackerProducer);

    // no kernel is in the new heap
    m_addedKernels.clear();
    m_addedKernelCount = 0;
    m_addedHashValues.clear();
    Refresh();
    m_isSipKernelLoaded = false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmISHBase::Refresh()
{
    // goes from back to front to handle tracker overflow
    while (m_destroyedTrackers.size())
    {
        FrameTrackerToken *trackerToken = m_destroyedTrackers.back();
        if (!trackerToken->IsExpired())
        {
            break;
        }
        MOS_RESOURCE *res = m_destroyedResources.back();
        m_osInterface->pfnUnlockResource(m_osInterface, res);
        m_osInterface->pfnFreeResourceWithFlag(m_osInterface, res, SURFACE_FLAG_ASSUME_NOT_IN_USE);

        m_destroyedResources.pop_back();
        m_destroyedTrackers.pop_back();

        MOS_FreeMemory(res);
        MOS_Delete(trackerToken);
    }

    return MOS_STATUS_SUCCESS;
}

void CmISHBase::Clean()
{
    m_offset = 0;
    m_addedKernels.clear();
    m_addedKernelCount = 0;
    Refresh();
    m_isSipKernelLoaded = false;
}

