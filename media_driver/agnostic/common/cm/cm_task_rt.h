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
//! \file      cm_task_rt.h
//! \brief     Declaration of CmTaskRT.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASKRT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASKRT_H_

#include "cm_task.h"
#include "cm_hal.h"
#include "cm_log.h"

namespace CMRT_UMD
{
class CmKernelRT;
class CmDeviceRT;

class CmTaskRT: public CmTask
{
public:
    static int32_t Create(CmDeviceRT *device,
                          uint32_t index,
                          uint32_t maxKernelCount,
                          CmTaskRT* &kernelArray);

    static int32_t Destroy(CmTaskRT *&kernelArray);

    CM_RT_API int32_t AddKernel(CmKernel *kernel);

    CM_RT_API int32_t Reset();

    CM_RT_API int32_t AddSync();

    CM_RT_API int32_t AddSyncEx(const CM_KERNEL_SYNC_CONFIG *config) { return CM_SUCCESS; }

    CM_RT_API int32_t SetPowerOption(PCM_POWER_OPTION powerOption);

    CM_RT_API int32_t AddConditionalEnd(SurfaceIndex *conditionalSurfaceIndex,
                                        uint32_t offset,
                                        CM_CONDITIONAL_END_PARAM *conditionalParam);

    CM_RT_API int32_t SetProperty(const CM_TASK_CONFIG &taskConfig);
    CM_RT_API int32_t GetProperty(CM_TASK_CONFIG &taskConfig);

    CM_RT_API int32_t AddKernelWithConfig(CmKernel *kernel, const CM_EXECUTION_CONFIG *config);

    uint32_t GetKernelCount();

    CmKernelRT *GetKernelPointer(uint32_t index);

    uint32_t GetIndexInTaskArray();

    bool IntegrityCheckKernelThreadspace();

    uint64_t GetSyncBitmap();

    uint64_t GetConditionalEndBitmap();

    CM_HAL_CONDITIONAL_BB_END_INFO *GetConditionalEndInfo();

    int32_t SetConditionalEndInfo(SurfaceIndex *index,
                                  uint32_t offset,
                                  CM_CONDITIONAL_END_PARAM *conditionalParam);

    PCM_POWER_OPTION GetPowerOption();

    int32_t AddKernelInternal( CmKernel *kernel,  const CM_EXECUTION_CONFIG *config);

    const CM_EXECUTION_CONFIG* GetKernelExecuteConfig() { return m_kernelExecuteConfig; };

#if CM_LOG_ON
    std::string Log();
#endif

protected:
    CmTaskRT(CmDeviceRT *device,
             uint32_t index,
             uint32_t maxKernelCount);

    ~CmTaskRT();

    int32_t Initialize();

#if USE_EXTENSION_CODE
    void AddKernelForGTPin(CmKernel *kernel);
#endif

    CmKernelRT **m_kernelArray;

    CmDeviceRT *m_device;

    uint32_t m_kernelCount;

    uint32_t m_maxKernelCount;

    uint32_t m_indexTaskArray;

    // Reserve a 64-bit variable to indicate if synchronization is insert for kernels.
    // 1 bit per kernel, 0 -- No sync, 1 -- Need sync
    // Up to 64 kernels supported
    uint64_t m_syncBitmap;

    // 64-bit variable to indicate if a conditional batch buffer end is inserted between kernels
    // 1 bit per kernel, 0 -- No conditional end, 1 -- Insert conditional end
    // Up to 64 kernels supported
    uint64_t m_conditionalEndBitmap;

    CM_HAL_CONDITIONAL_BB_END_INFO
    m_conditionalEndInfo[CM_MAX_CONDITIONAL_END_CMDS];

    CM_POWER_OPTION m_powerOption;

    CM_TASK_CONFIG m_taskConfig;

    CM_EXECUTION_CONFIG m_kernelExecuteConfig[CM_MAX_KERNELS_PER_TASK]; // replace numOfWalkers in CM_TASK_CONFIG.

private:
    CmTaskRT(const CmTaskRT &other);
    CmTaskRT &operator=(const CmTaskRT &other);
};
};  // namespace;

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASKRT_H_
