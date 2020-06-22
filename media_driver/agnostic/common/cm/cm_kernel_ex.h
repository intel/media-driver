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
//! \file      cm_kernel_ex.h
//! \brief     Contains Class CmKernelEx  definitions 
//!
#pragma once

#include "cm_kernel_rt.h"
#include "cm_group_space.h"

class CmSurfaceState;
class CmSSH;
class CmISHBase;
class CmMediaState;
namespace CMRT_UMD
{
class CmThreadSpaceRT;
class CmThreadGroupSpace;
};
class CmKernelEx : public CMRT_UMD::CmKernelRT
{
public:
    CmKernelEx(CMRT_UMD::CmDeviceRT *device,
               CMRT_UMD::CmProgramRT *program,
               uint32_t kernelIndex,
               uint32_t kernelSeqNum):
        CMRT_UMD::CmKernelRT(device, program, kernelIndex, kernelSeqNum),
        m_indexMap(nullptr),
        m_flatArgs(nullptr),
        m_flatArgCount(0),
        m_data(nullptr),
        m_surfaceInArg(nullptr),
        m_curbe(nullptr),
        m_explicitCurbeSize(0),
        m_curbeSize(0),
        m_curbeSizePerThread(0),
        m_curbeSizeCrossThread(0),
        m_propertyIndexes(nullptr),
        m_cmSurfIndexes(nullptr),
        m_hashValue(0),
        m_dummyThreadSpace(nullptr),
        m_dummyThreadGroupSpace(nullptr),
        m_ish(nullptr),
        m_indexInIsh(-1),
        m_offsetInIsh(0)
    {
    }

    virtual ~CmKernelEx();

    int32_t SetKernelArg(uint32_t index, size_t size, const void * value);

    int32_t SetSurfaceBTI(CMRT_UMD::SurfaceIndex *surfIndex, uint32_t bti);

    int32_t SetSamplerBTI(CMRT_UMD::SamplerIndex* sampler, uint32_t nIndex);

    int32_t SetStaticBuffer(uint32_t index, const void *value);

    inline uint32_t GetCurbeSize() {return m_curbeSize; }

    inline uint32_t GetCurbeSizePerThread() {return m_curbeSizePerThread; }

    inline uint32_t GetCurbeSizeCrossThread() {return m_curbeSizeCrossThread; }

    inline uint8_t* GetCurbe() {return m_curbe; }

    uint32_t GetMaxBteNum();

    MOS_STATUS AllocateCurbe(); // for media walker

    MOS_STATUS AllocateCurbeAndFillImplicitArgs(CMRT_UMD::CmThreadGroupSpace *globalGroupSpace); // for gpgpu/compute walker

    MOS_STATUS UpdateCurbe(CmSSH *ssh, CmMediaState *mediaState, uint32_t kernelIdx);

    MOS_STATUS UpdateSWSBArgs(CMRT_UMD::CmThreadSpaceRT *threadSpace);

    MOS_STATUS UpdateFastTracker(uint32_t trackerIndex, uint32_t tracker);

    MOS_STATUS LoadReservedSurfaces(CmSSH *ssh);

    MOS_STATUS LoadReservedSamplers(CmMediaState *mediaState, uint32_t kernelIdx);

    MOS_STATUS GetSamplerCount(uint32_t *count3D, uint32_t *countAVS);

    CMRT_UMD::CmThreadSpaceRT* GetThreadSpaceEx();

    CMRT_UMD::CmThreadGroupSpace* GetThreadGroupSpaceEx();

    void SurfaceDumpEx(uint32_t kernelNumber, int32_t taskId);

    bool IsFastPathSupported();

    // interfaces with ISH
    inline void Recorded(CmISHBase *ish, int index, uint32_t offset)
    {
        m_ish = ish;
        m_indexInIsh = index;
        m_offsetInIsh = offset;
    }

    inline void GetRecordedInfo(CmISHBase **ish, int *index)
    {
        *ish = m_ish;
        *index = m_indexInIsh;
    }

    inline uint32_t GetOffsetInIsh() {return m_offsetInIsh; }

    inline void GetNativeKernel(uint8_t **kernel, uint32_t *size)
    {
        *kernel = (uint8_t *)m_binary;
        *size = m_binarySize;
    }

    inline uint32_t GetBarrierMode() {return m_barrierMode; }

    inline std::map<int, CmSurfaceState *>& GetReservedSurfaceBteIndex() {return m_reservedSurfaceBteIndexes; }

    inline std::map<int, void *>& GetReservedSamplerBteIndex() {return m_reservedSamplerBteIndexes; }

    inline uint64_t GetHashValue() {return m_hashValue; }

protected:
    int32_t Initialize(const char *kernelName, const char *options);

    bool IsSurface(uint16_t kind);

    CM_ARG_KIND ToArgKind(CMRT_UMD::CmSurface *surface);

    inline bool ArgArraySupported(uint16_t kind) {return IsSurface(kind) || kind == ARG_KIND_SAMPLER; }

    CmSurfaceState* GetSurfaceState(CMRT_UMD::CmSurface *surface, uint32_t index);

    void* GetSamplerParam(uint32_t index);

    struct _CmArg
    {
        uint32_t offset;
        uint16_t kind;
        uint16_t isaKind;
        uint16_t unitSize;
        uint16_t payloadOffset;
        uint16_t sizeInCurbe;
        bool isSet;
    };

    uint32_t *m_indexMap;
    _CmArg *m_flatArgs;
    uint32_t m_flatArgCount;
    uint8_t *m_data;
    uint8_t *m_surfaceInArg;

    uint8_t *m_curbe;
    uint32_t m_explicitCurbeSize;
    uint32_t m_curbeSize;
    uint32_t m_curbeSizePerThread;
    uint32_t m_curbeSizeCrossThread;

    uint8_t *m_propertyIndexes;
    uint32_t *m_cmSurfIndexes;

    uint64_t m_hashValue;

    // reserved an internal space for setThreadCount
    CMRT_UMD::CmThreadSpace *m_dummyThreadSpace;
    CMRT_UMD::CmThreadGroupSpace *m_dummyThreadGroupSpace;

    //recorded info in ish
    CmISHBase *m_ish;
    int m_indexInIsh;
    uint32_t m_offsetInIsh;

    // reserved surface bte index
    std::map<int, CmSurfaceState *>m_reservedSurfaceBteIndexes;

    // reserved sampler bte index
    std::map<int, void *>m_reservedSamplerBteIndexes;
};
