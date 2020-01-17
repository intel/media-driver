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
//! \file      cm_buffer_rt.h
//! \brief     Declaration of CmBuffer_RT.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMBUFFERRT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMBUFFERRT_H_

#include "cm_buffer.h"
#include "cm_surface.h"

namespace CMRT_UMD
{
class CmBuffer_RT: public CmBuffer,
                   public CmBufferUP,
                   public CmBufferSVM,
                   public CmSurface,
                   public CmBufferStateless
{
public:
    static int32_t Create(uint32_t index,
                          uint32_t handle,
                          size_t size,
                          bool isCmCreated,
                          CmSurfaceManager *surfaceManager,
                          uint32_t bufferType,
                          bool isCMRTAllocatedSVM,
                          void *sysMem,
                          CmBuffer_RT* &surface,
                          bool isConditionalBuffer,
                          uint32_t comparisonValue,
                          uint64_t gfxMem,
                          bool enableCompareMask = false);

    CM_RT_API int32_t ReadSurface(unsigned char* sysMem,
                                  CmEvent* event,
                                  uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t WriteSurface(const unsigned char* sysMem,
                                   CmEvent* event,
                                   uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);


    int32_t ReadBuffer(unsigned char* sysMem,
                                 CmEvent* event,
                                 uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                                 size_t offset = 0);

    int32_t WriteBuffer(const unsigned char* sysMem,
                                  CmEvent* event,
                                  uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                                  size_t offset = 0);

    CM_RT_API int32_t InitSurface(const uint32_t initValue, CmEvent *event);

    CM_RT_API int32_t GetIndex(SurfaceIndex *&index);

    int32_t GetHandle(uint32_t &handle);

    //NOT depend on RTTI::dynamic_cast
    CM_RT_API CM_ENUM_CLASS_TYPE Type() const
    { return CM_ENUM_CLASS_TYPE_CMBUFFER_RT; };

    int32_t SetMemoryObjectControl(MEMORY_OBJECT_CONTROL memCtrl,
                                   MEMORY_TYPE memType,
                                   uint32_t age);

    CM_RT_API int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl);

    CMRT_UMD_API int32_t
    SetResourceUsage(const MOS_HW_RESOURCE_DEF mosUsage);

    CM_RT_API int32_t GetAddress(void* &addr);

    CM_RT_API int32_t
    SetSurfaceStateParam(SurfaceIndex *surfIndex,
                         const CM_BUFFER_STATE_PARAM *bufferStateParam);

    CM_RT_API int32_t GetGfxAddress(uint64_t &gfxAddr);

    CM_RT_API int32_t GetSysAddress(void* &sysAddr);

    int32_t UpdateResource(MOS_RESOURCE *resource);

    size_t GetSize() { return m_size; }

    void SetSize(size_t size);

    bool IsUpSurface();

    bool IsSVMSurface();

    bool IsCMRTAllocatedSVMBuffer();

    bool IsConditionalSurface();

    uint32_t GetConditionalCompareValue();

    bool IsCompareMaskEnabled();

    uint32_t GetBufferType() { return m_bufferType; }

    int32_t CreateBufferAlias(SurfaceIndex *&aliasSurfIndex);

    int32_t GetNumAliases(uint32_t &numAliases);

    void Log(std::ostringstream &oss);

    void DumpContent(uint32_t kernelNumber,
                     char *kernelName,
                     int32_t taskId,
                     uint32_t argIndex,
                     uint32_t vectorIndex);

    int32_t UpdateProperty(uint32_t size);

protected:
    CmBuffer_RT(uint32_t handle,
                size_t size,
                bool isCmCreated,
                CmSurfaceManager *surfaceManager,
                uint32_t bufferType,
                bool isCMRTAllocatedSVM,
                void *sysMem,
                bool isConditionalBuffer,
                uint32_t comparisonValue,
                uint64_t gfxAddr,
                bool enableCompareMask = false);

    ~CmBuffer_RT();

    int32_t Initialize(uint32_t index);

    uint32_t m_handle;

    size_t m_size;

    uint32_t m_bufferType;  // SURFACE_TYPE_BUFFER, SURFACE_TYPE_BUFFER_UP,
                              // SURFACE_TYPE_BUFFER_SVM

    void *m_sysMem;  // start address of BufferUP/BufferSVM/BufferStatelss in CPU memory space
    uint64_t m_gfxMem;  // start address of buffer in graphics memory space

    bool m_isCMRTAllocatedSVMBuffer;  //0--User provided SVM buffer, 1--CMRT allocated SVM buffer

    bool m_isConditionalBuffer;

    uint32_t m_comparisonValue;  // value used for conditional batch buffer end

    bool m_enableCompareMask;

    uint32_t m_numAliases;  // number of alias indexes

    SurfaceIndex *m_aliasIndexes[CM_HAL_MAX_NUM_BUFFER_ALIASES];

private:
    CmBuffer_RT(const CmBuffer_RT& other);
    CmBuffer_RT& operator=(const CmBuffer_RT& other);
};
};

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMBUFFERRT_H_
