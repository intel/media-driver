/*
* Copyright (c) 2018, Intel Corporation
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
//! \file      cm_ssh.h
//! \brief     Contains Class CmSSH  definitions 
//!
#pragma once

#include "cm_hal.h"
#include "heap_manager.h"
#include <list>

class CmKernelEx;
namespace CMRT_UMD
{
class CmThreadGroupSpace;
};

class CmMediaState
{
public:
    CmMediaState(CM_HAL_STATE *cmhal);
    ~CmMediaState();

    MOS_STATUS Initialize(HeapManager *heapMgr);

    MOS_STATUS Allocate(CmKernelEx **kernels, int count, uint32_t trackerIndex, uint32_t trackerID);

    inline MOS_RESOURCE* GetHeapResource() {return m_memoryBlock.GetResource(); }

    inline uint32_t GetCurbeOffset() {return m_curbeOffsetInternal + m_memoryBlock.GetOffset();}

    inline uint32_t GetCurbeOffset(uint32_t kernelIndex) {return m_curbeOffsets[kernelIndex] + m_curbeOffsetInternal + m_memoryBlock.GetOffset();}

    inline uint32_t GetMediaIDOffset() {return m_mediaIDOffsetInternal + m_memoryBlock.GetOffset();}

    inline uint32_t GetHeapSize() {return m_memoryBlock.GetHeapSize(); }

    inline uint32_t GetCurbeSize() {return m_totalCurbeSize; }

    inline uint32_t GetMediaIDSize() {return m_totalMediaIDSize;}

    inline uint32_t GetSamplerHeapOffset(uint32_t kernelIndex) {return m_samplerOffsets[kernelIndex] + m_samplerHeapOffsetInternal + m_memoryBlock.GetOffset();}

    inline uint32_t GetSamplerCount(uint32_t kernelIndex) {return m_samplerCount[kernelIndex]; }

    inline uint32_t GetScratchSizePerThread() {return m_scratchSizePerThread; }

    inline uint32_t GetScratchSpaceOffset() {return m_scratchSpaceOffsetExternal; }

    MOS_STATUS LoadCurbe(CmKernelEx *kernel, int index);

    MOS_STATUS LoadCurbe(uint8_t *curbe, uint32_t size, int index);

    MOS_STATUS LoadMediaID(CmKernelEx *kernel, int index, uint32_t btOffset, CMRT_UMD::CmThreadGroupSpace *threadGroupSpace = nullptr);

    int AddSampler(void *samplerParam, int index, int bteIndex = -1);

    MOS_STATUS Submit();

    void Dump();

protected:

    MOS_STATUS PrepareMemoryBlock(uint32_t size, uint32_t trackerIndex, uint32_t trackerID);

    uint32_t UpdateHeapSizeAndOffsets(CmKernelEx *kernel, uint32_t kernelIdx);
    
    enum _BlockState
    {
        _Empty,
        _Allocated,
        _Submitted
    };
    
    CM_HAL_STATE *m_cmhal;
    HeapManager *m_heapMgr;

    MemoryBlock m_memoryBlock;

    uint32_t m_curbeOffsetInternal;
    uint32_t m_mediaIDOffsetInternal;
    uint32_t m_samplerHeapOffsetInternal;
    uint32_t m_scratchSpaceOffsetExternal;

    uint32_t m_totalCurbeSize;
    uint32_t m_totalMediaIDSize;
    uint32_t m_totalSamplerHeapSize;
    uint32_t m_totalScratchSpaceSize;

    uint32_t m_curbeOffsets[CM_MAX_KERNELS_PER_TASK];
    uint32_t m_samplerOffsets[CM_MAX_KERNELS_PER_TASK];
    uint32_t m_next3dSamplerOffsets[CM_MAX_KERNELS_PER_TASK];
    uint32_t m_nextAvsSamplerOffsets[CM_MAX_KERNELS_PER_TASK];
    uint32_t m_nextIndStateOffsets[CM_MAX_KERNELS_PER_TASK];
    uint32_t m_samplerCount[CM_MAX_KERNELS_PER_TASK];
    uint32_t m_mediaIDSize;
    uint32_t m_scratchSizePerThread;

    _BlockState m_state;

    const uint32_t m_3dSamplerElementSize = 16;
    const uint32_t m_avsSamplerElementSize = 2048;

};
