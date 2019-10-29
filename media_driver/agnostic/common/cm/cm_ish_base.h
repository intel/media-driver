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
//! \file      cm_ish_base.h
//! \brief     Contains Class CmISHBase  definitions
//!
#pragma once

#include "mos_os.h"
#include <list>
#include "igfxfmid.h"
#include "frame_tracker.h"

class CmKernelEx;
typedef struct _CM_HAL_STATE CM_HAL_STATE;

class CmISHBase
{
public:
    MOS_STATUS Initialize(CM_HAL_STATE *cmhal, FrameTrackerProducer *trackerProducer);

    MOS_STATUS LoadKernels(CmKernelEx **kernels, int count);

    inline MOS_RESOURCE* GetResource() {return m_resource; }

    uint32_t GetSize() {return m_size; }

    MOS_STATUS Refresh();

    inline void Submit(uint32_t trackerIndex, uint32_t tracker) {m_lastTrackerToken->Merge(trackerIndex, tracker) ; }

    inline uint32_t GetSipKernelOffset() {return m_sipKernelOffset; }

    void Clean();

protected:
    CmISHBase();
    virtual ~CmISHBase();

    MOS_STATUS ExpandHeapSize(uint32_t extraSize);

    virtual MOS_STATUS CreateSipKernel(CM_HAL_STATE *state) = 0;

    MOS_INTERFACE *m_osInterface;
    MOS_RESOURCE *m_resource;
    uint8_t *m_lockedData;
    uint32_t m_size;
    uint32_t m_offset;

    // sync resources
    //uint32_t *m_latestTracker;
    //uint32_t m_lastTracker;
    FrameTrackerProducer *m_trackerProducer;
    FrameTrackerToken *m_lastTrackerToken;

    // destroyed buffers
    std::list<MOS_RESOURCE *> m_destroyedResources;
    std::list<FrameTrackerToken *> m_destroyedTrackers;

    //recorded info
    std::vector<CmKernelEx *> m_addedKernels;
    uint32_t m_addedKernelCount;
    std::map<uint64_t, uint32_t> m_addedHashValues;

    // settings
    const uint32_t m_initSize = 0x80000;
    const uint32_t m_expandStep = 0x80000;
    const uint32_t m_kernelAlign = 64;
    const uint32_t m_kernelPadding = 128;
    
    // SIP kernels
    bool m_isSipKernelLoaded;
    uint8_t *m_sipKernel;
    uint32_t m_sipKernelSize;
    uint32_t m_sipKernelOffset;
};
