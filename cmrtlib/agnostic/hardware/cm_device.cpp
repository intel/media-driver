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
#include "cm_device.h"
#include <cstdarg>
#include "cm_debug.h"
#include "cm_mem.h"
#if USE_EXTENSION_CODE
#include "cm_gtpin_external_interface.h"
#endif
#include "cm_printf_host.h"
#include "cm_surface_manager.h"
#include "cm_queue.h"
#include "cm_perf_statistics.h"
#include "cm_timer.h"

class CmBuffer;
class CmBufferUP;
class CmBufferSVM;
class CmBufferStateless;

#if MDF_PROFILER_ENABLED
CmPerfStatistics gCmPerfStatistics;  // global instance to record API's perf
#endif

CM_RT_API int32_t CmDevice_RT::CreateBuffer(uint32_t size, CmBuffer* &buffer)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateBuffer(size, buffer);
}

CM_RT_API int32_t CmDevice_RT::CreateBufferUP(uint32_t size, void* sysMem, CmBufferUP* &buffer)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateBufferUP( size, sysMem, buffer);
}

CM_RT_API int32_t CmDevice_RT::DestroySurface(CmBuffer* &buffer)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->DestroyBuffer(buffer);
}

CM_RT_API int32_t CmDevice_RT::DestroyBufferUP(CmBufferUP* &buffer)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->DestroyBufferUP(buffer);
}

//!
//! Create a CmSurface2D
//! Input :
//!     Surface width, height and format;
//!     Reference to the pointer to the CmSurface2D .
//! Output:
//!     CM_SUCCESS if the CmSurface2D is successfully created;
//!     CM_OUT_OF_HOST_MEMORY if out of system memory;
//!     CM_SURFACE_ALLOCATION_FAILURE if surface creation fails;
//!     CM_FAILURE otherwise;
//!NOTES: general API
CM_RT_API int32_t CmDevice_RT::CreateSurface2D(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, CmSurface2D* & surface )
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateSurface2D( width,  height,  format, surface);;
}

CM_RT_API int32_t CmDevice_RT::CreateSurface2DUP( uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, void* sysMem, CmSurface2DUP* &surface )
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateSurface2DUP( width, height, format, sysMem, surface);
}

CM_RT_API int32_t CmDevice_RT::DestroySurface2DUP( CmSurface2DUP* &surface)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->DestroySurface2DUP(surface);
}

CM_RT_API int32_t CmDevice_RT::DestroySurface( CmSurface2D* &surface2d)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->DestroySurface(surface2d);
}

CM_RT_API int32_t CmDevice_RT::GetSurface2DInfo( uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, uint32_t & pitch, uint32_t & physicalSize)
{
    INSERT_PROFILER_RECORD();

    CM_GETSURFACE2DINFO_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_GETSURFACE2DINFO_PARAM ) );
    inParam.width = width;
    inParam.height = height;
    inParam.format = format;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_GETSURFACE2DINFO,
                                      &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);

    pitch = inParam.pitch;
    physicalSize = inParam.physicalSize;
    return CM_SUCCESS;
}

//!
//! Load the code of one or more kernels in common ISA.
//! Jit-ting of the common ISA code can happen either in LoadProgram or CreateKernel.
//! Jit options set in LoadProgram are for all kernels within the code.
//! Jit options set in CreateKernel are for the specific kernel, overwriting the jit options set in program level. LoadProgram can be called multiple times for the same code.
//! Jit options is a null terminated string. string length ( excluding the null terminater )shoudl be
//! less than 512 (CM_MAX_OPTION_SIZE_IN_BYTE ). Otherwise the option is ignored.
//! Each time a now CmProgram object is generated.
//!
//! Input:
//!     1) Pointer pointing to the common ISA code
//!     2) code size in byte
//!     3) Reference to the pointer to the CmProgram.
//!     4) Jit options. This argument is optional.
//! Output:
//!     CM_SUCCESS if the CmProgram is successfully created;
//!     CM_INVALID_KERNELS if any kernel is invalid;
//!     CM_OUT_OF_HOST_MEMORY if out of system memory;
//!     CM_FAILURE otherwise;
//!

CM_RT_API int32_t CmDevice_RT::LoadProgram( void* commonISACode, const uint32_t size, CmProgram*& program,  const char* options )
{
    INSERT_PROFILER_RECORD();

    if( (commonISACode == nullptr) || (size == 0) )
    {
        CmAssert( 0 );
        return CM_INVALID_COMMON_ISA;
    }

    return CreateProgram(commonISACode, size, program, options);
}

int32_t CmDevice_RT::CreateProgram(void* commonISACode,
                                   const uint32_t size,
                                   CmProgram*& program,
                                   const char* options)
{
    CM_LOADPROGRAM_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_LOADPROGRAM_PARAM ) );
    inParam.cisaCode = commonISACode;
    inParam.cisaCodeSize = size;
    inParam.options = (char *)options;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_LOADPROGRAM,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);

    program = (CmProgram *)inParam.cmProgramHandle;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::DestroyProgram(CmProgram* & program )
{
    INSERT_PROFILER_RECORD();

    if( program == nullptr )
    {
        return CM_FAILURE;
    }

    CM_DESTROYPROGRAM_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYPROGRAM_PARAM ) );
    inParam.cmProgramHandle = program;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYPROGRAM,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);

    program = nullptr;

    return CM_SUCCESS;
}

//!
//! Create a CmKernel_RT. Each kernel can run in multiple threads.
//! If CreateKernel is called multiple times using the same CmProgram,
//! the same kernel name, and the same options, only in the first time a new CmKernel_RT is generated,
//! all following calls only return the existing CmKernel_RT.
//! Otherwise, each time a new CmKernel_RT is generated.
//! Input :
//!     1) CM Program from which the kernel is created
//!     2) CM kernel function (genx_main) name
//!     3) Reference to the pointer to the CmKernel_RT .
//!     4) Jit options for this kernel, overwriting the jit options in
//!        CmProgram level for this specific kernel. This argument is optional.
//! Output:
//!     CM_SUCCESS if the CmKernel_RT is successfully created;
//!     CM_OUT_OF_HOST_MEMORY if out of system memory;
//!     CM_FAILURE otherwise;
//!
CM_RT_API int32_t CmDevice_RT::CreateKernel( CmProgram* program, const char* kernelName, CmKernel* & kernel, const char* options )
{
    INSERT_PROFILER_RECORD();

    if(program == nullptr)
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    CM_CREATEKERNEL_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATEKERNEL_PARAM ) );
    inParam.cmProgramHandle = program;
    inParam.kernelName = (char*)kernelName;
    inParam.options = (char *)options;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_CREATEKERNEL,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    kernel = (CmKernel *)inParam.cmKernelHandle; // Got Object from CMRT@UMD directly.

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::DestroyKernel( CmKernel*& kernel)
{
    INSERT_PROFILER_RECORD();

    CM_DESTROYKERNEL_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYKERNEL_PARAM ) );
    inParam.cmKernelHandle = kernel;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYKERNEL,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    kernel = nullptr;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateTask(CmTask *& task)
{
    INSERT_PROFILER_RECORD();

    CM_CREATETASK_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATETASK_PARAM ) );

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_CREATETASK,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    task = (CmTask *)inParam.cmTaskHandle;

#if USE_EXTENSION_CODE
    GTPIN_MAKER_FUNCTION(CmrtCodeMarkerForGTPin_CreateTask(this, task));
#endif
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::DestroyTask( CmTask*& task)
{
    INSERT_PROFILER_RECORD();

#if USE_EXTENSION_CODE
    GTPIN_MAKER_FUNCTION(CmrtCodeMarkerForGTPin_DestroyTask(this, task));
#endif

    CM_DESTROYTASK_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYTASK_PARAM ) );
    inParam.cmTaskHandle = task;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYTASK,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    task = nullptr;

    return CM_SUCCESS;

}

//!
//! Create a task queue, CmQueue_RT. It is an in-order queue of tasks. Each task can
//! have multiple kernels running concurrently, each kernel can run in multiple threads.
//! For now only one CmQueue_RT is supported. Trying to create a second CmQueue_RT will fail.
//! Input :
//!     Reference to the pointer to the CmQueue_RT .
//! Output:
//!     CM_SUCCESS if the CmQueue_RT is successfully created;
//!     CM_OUT_OF_HOST_MEMORY if out of system memory;
//!     CM_FAILURE otherwise;
//!
CM_RT_API int32_t CmDevice_RT::CreateQueue(CmQueue* &queue)
{
    INSERT_PROFILER_RECORD();

    CM_QUEUE_CREATE_OPTION option = CM_DEFAULT_QUEUE_CREATE_OPTION;
    int32_t result = CreateQueueEx(queue, option);

#if USE_EXTENSION_CODE
    GTPIN_MAKER_FUNCTION(CmrtCodeMarkerForGTPin_CreateQueue(this, queue));
#endif

    return result;
}

CM_RT_API int32_t
CmDevice_RT::CreateQueueEx(CmQueue* &queue,
                           CM_QUEUE_CREATE_OPTION queueCreateOption)
{
    INSERT_PROFILER_RECORD();
    m_criticalSectionQueue.Acquire();

    CmQueue_RT *queueRT = nullptr;
    if (CM_QUEUE_TYPE_RENDER == queueCreateOption.QueueType)
    {
        for (auto iter = m_queue.begin(); iter != m_queue.end(); ++iter)
        {
            CM_QUEUE_TYPE queueType = (*iter)->GetQueueOption().QueueType;
            unsigned int gpuContext = (*iter)->GetQueueOption().GPUContext;
            if (queueType == CM_QUEUE_TYPE_RENDER
                && gpuContext == queueCreateOption.GPUContext)
            {
                queue = (*iter);
                m_criticalSectionQueue.Release();
                return CM_SUCCESS;
            }
        }
    }

    int32_t result = CmQueue_RT::Create(this, queueRT, queueCreateOption);
    if (CM_SUCCESS != result || nullptr == queueRT)
    {
        CmAssert(0);
        CmDebugMessage(("Failed to create queue!"));
        m_criticalSectionQueue.Release();
        return result;
    }
    m_queue.push_back(queueRT);
    queue = queueRT;
    m_criticalSectionQueue.Release();

    return result;
}

CM_RT_API int32_t CmDevice_RT::CreateThreadSpace( uint32_t width, uint32_t height, CmThreadSpace* &threadSpace)
{
    INSERT_PROFILER_RECORD();

    CM_CREATETHREADSPACE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATETHREADSPACE_PARAM ) );
    inParam.tsWidth  = width;
    inParam.tsHeight = height;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_CREATETHREADSPACE,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    threadSpace = (CmThreadSpace *)inParam.cmTsHandle;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::DestroyThreadSpace( CmThreadSpace* &threadSpace)
{
    INSERT_PROFILER_RECORD();

    CM_DESTROYTHREADSPACE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYTHREADSPACE_PARAM ) );
    inParam.cmTsHandle = threadSpace;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYTHREADSPACE,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    threadSpace = nullptr;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateVmeSurfaceG7_5( CmSurface2D* currentSurface,
                                                 CmSurface2D **forwardSurfaceArray,
                                                 CmSurface2D **backwardSurfaceArray,
                                                 const uint32_t surfaceCountForward,
                                                 const uint32_t surfaceCountBackward,
                                                 SurfaceIndex* & vmeSurfaceIndex )
{
    INSERT_PROFILER_RECORD();

    return CreateVmeSurface( currentSurface, forwardSurfaceArray, backwardSurfaceArray, surfaceCountForward, surfaceCountBackward, vmeSurfaceIndex, CM_FN_CMDEVICE_CREATEVMESURFACEG7_5 );
}

CM_RT_API int32_t CmDevice_RT::DestroyVmeSurfaceG7_5( SurfaceIndex* & vmeSurfaceIndex )
{
    INSERT_PROFILER_RECORD();

    return DestroyVmeSurface( vmeSurfaceIndex );
}

CM_RT_API int32_t CmDevice_RT::SetVmeSurfaceStateParam(SurfaceIndex* vmeIndex, CM_VME_SURFACE_STATE_PARAM *surfStateParam)
{
    INSERT_PROFILER_RECORD();

    if(vmeIndex == nullptr || surfStateParam == nullptr)
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    CM_CONFIGVMESURFACEDIMENSION_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CONFIGVMESURFACEDIMENSION_PARAM ) );
    inParam.cmVmeSurfHandle = (void *)vmeIndex;
    inParam.surfDimPara = surfStateParam;

    int32_t result
        = OSALExtensionExecute(CM_FN_CMDEVICE_CONFIGVMESURFACEDIMENSION,
                               &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);

    return inParam.returnValue;
}

CM_RT_API int32_t CmDevice_RT::CreateSampler( const CM_SAMPLER_STATE& samplerState, CmSampler* &sampler )
{
    INSERT_PROFILER_RECORD();

    CM_CREATESAMPLER_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATESAMPLER_PARAM ) );
    inParam.samplerState = samplerState;

    int32_t result = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESAMPLER,
                                          &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    sampler = (CmSampler *)inParam.cmSamplerHandle;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateSamplerEx( const CM_SAMPLER_STATE_EX& samplerState, CmSampler* &sampler )
{
    INSERT_PROFILER_RECORD();

    CM_CREATESAMPLER_PARAM_EX inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATESAMPLER_PARAM_EX ) );
    inParam.samplerState = samplerState;

    int32_t result = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESAMPLER_EX,
                                          &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    sampler = (CmSampler *)inParam.cmSamplerHandle;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::DestroySampler( CmSampler* &sampler )
{
    INSERT_PROFILER_RECORD();

    CM_DESTROYSAMPLER_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYSAMPLER_PARAM ) );
    inParam.cmSamplerHandle = sampler;

    int32_t result = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYSAMPLER,
                                          &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    sampler = nullptr;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateThreadGroupSpace( uint32_t threadSpaceWidth, uint32_t threadSpaceHeight, uint32_t groupSpaceWidth, uint32_t groupSpaceHeight, CmThreadGroupSpace* &threadGroupSpace)
{
    INSERT_PROFILER_RECORD();

    CM_CREATETGROUPSPACE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATETGROUPSPACE_PARAM ) );
    inParam.thrdSpaceWidth  = threadSpaceWidth;
    inParam.thrdSpaceHeight = threadSpaceHeight;
    inParam.thrdSpaceDepth = 1;
    inParam.grpSpaceWidth   = groupSpaceWidth;
    inParam.grpSpaceHeight  = groupSpaceHeight;
    inParam.grpSpaceDepth   = 1;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_CREATETHREADGROUPSPACE,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    threadGroupSpace = (CmThreadGroupSpace *)inParam.cmGrpSpaceHandle;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateThreadGroupSpaceEx(uint32_t threadSpaceWidth, uint32_t threadSpaceHeight, uint32_t threadSpaceDepth, uint32_t groupSpaceWidth, uint32_t groupSpaceHeight, uint32_t groupSpaceDepth, CmThreadGroupSpace* &threadGroupSpace)
{
    INSERT_PROFILER_RECORD();

    CM_CREATETGROUPSPACE_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_CREATETGROUPSPACE_PARAM));
    inParam.thrdSpaceWidth = threadSpaceWidth;
    inParam.thrdSpaceHeight = threadSpaceHeight;
    inParam.thrdSpaceDepth = threadSpaceDepth;
    inParam.grpSpaceWidth = groupSpaceWidth;
    inParam.grpSpaceHeight = groupSpaceHeight;
    inParam.grpSpaceDepth = groupSpaceDepth;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_CREATETHREADGROUPSPACE,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    threadGroupSpace = (CmThreadGroupSpace *)inParam.cmGrpSpaceHandle;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::DestroyThreadGroupSpace(CmThreadGroupSpace* &threadGroupSpace)
{
    INSERT_PROFILER_RECORD();

    CM_DESTROYTGROPUSPACE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYTGROPUSPACE_PARAM ) );
    inParam.cmGrpSpaceHandle = threadGroupSpace;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYTHREADGROUPSPACE,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    threadGroupSpace = nullptr;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::GetCaps(CM_DEVICE_CAP_NAME capName, size_t& capValueSize, void* capValue )
{
    INSERT_PROFILER_RECORD();

    CM_GETCAPS_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_GETCAPS_PARAM ) );
    inParam.capName         = capName;
    inParam.capValueSize    = (uint32_t)capValueSize;
    inParam.capValue        = capValue;

    int32_t result = OSALExtensionExecute(CM_FN_CMDEVICE_GETCAPS,
                                          &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateSurface3D(uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format, CmSurface3D* & surface )
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateSurface3D( width,  height,  depth,  format, surface);
}

CM_RT_API int32_t CmDevice_RT::CreateSampler8x8(const CM_SAMPLER_8X8_DESCR  &samplerDescriptor, CmSampler8x8* &sampler)
{
    INSERT_PROFILER_RECORD();

    if((samplerDescriptor.stateType == CM_SAMPLER8X8_AVS && samplerDescriptor.avs == nullptr) ||
        (samplerDescriptor.stateType == CM_SAMPLER8X8_CONV && samplerDescriptor.conv == nullptr) ||
        (samplerDescriptor.stateType == CM_SAMPLER8X8_CONV1DH && samplerDescriptor.conv == nullptr) ||
        (samplerDescriptor.stateType == CM_SAMPLER8X8_CONV1DV && samplerDescriptor.conv == nullptr) ||
        (samplerDescriptor.stateType == CM_SAMPLER8X8_MISC && samplerDescriptor.misc == nullptr) ||
        (samplerDescriptor.stateType == CM_SAMPLER8X8_NONE && samplerDescriptor.conv != nullptr) ||
        sampler != nullptr)
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    CM_CREATESAMPLER8x8_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATESAMPLER8x8_PARAM ) );
    inParam.sampler8x8Desc = samplerDescriptor;

    int32_t result = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESAMPLER8X8,
                                          &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    sampler = (CmSampler8x8 *)inParam.cmSampler8x8Handle;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::DestroySampler8x8( CmSampler8x8 *& sampler8x8 )
{
    INSERT_PROFILER_RECORD();

    CM_DESTROYSAMPLER8x8_PARAM inParam;

    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYSAMPLER8x8_PARAM ) );
    inParam.cmSampler8x8Handle = sampler8x8;

    int32_t result = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYSAMPLER8X8,
                                          &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    sampler8x8 = nullptr;

    return CM_SUCCESS;
}

 CM_RT_API int32_t CmDevice_RT::CreateSampler8x8Surface(CmSurface2D* surface2d, SurfaceIndex* &sampler8x8SurfaceIndex, CM_SAMPLER8x8_SURFACE surfaceType, CM_SURFACE_ADDRESS_CONTROL_MODE addressControl)
 {
     INSERT_PROFILER_RECORD();

    CmSurface2D* currentRT = static_cast< CmSurface2D* >( surface2d );
    if( ! currentRT )  {
        CmAssert( 0 );
        return CM_FAILURE;
    }

    CM_CREATESAMPLER8x8SURF_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATESAMPLER8x8SURF_PARAM ) );
    inParam.cmSurf2DHandle   = currentRT;
    inParam.cmSampler8x8Type = surfaceType;
    inParam.sampler8x8Mode   = addressControl;

    int32_t result
        = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE,
                               &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    sampler8x8SurfaceIndex = inParam.cmSurfIndexHandle;

    return CM_SUCCESS;
 }

 CM_RT_API int32_t CmDevice_RT::CreateSampler8x8SurfaceEx(CmSurface2D* surface2d, SurfaceIndex* &sampler8x8SurfaceIndex, CM_SAMPLER8x8_SURFACE surfaceType, CM_SURFACE_ADDRESS_CONTROL_MODE addressControl, CM_FLAG* flag)
 {
     INSERT_PROFILER_RECORD();

     CmSurface2D* currentRT = static_cast< CmSurface2D* >(surface2d);
     if (!currentRT)  {
         CmAssert(0);
         return CM_FAILURE;
     }

     CM_CREATESAMPLER8x8SURFEX_PARAM inParam;
     CmSafeMemSet(&inParam, 0, sizeof(inParam));
     inParam.cmSurf2DHandle = currentRT;
     inParam.cmSampler8x8Type = surfaceType;
     inParam.sampler8x8Mode = addressControl;
     inParam.flag = flag;

     int32_t result
         = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE_EX,
                                &inParam, sizeof(inParam));

     CHK_FAILURE_RETURN(result);
     CHK_FAILURE_RETURN(inParam.returnValue);
     sampler8x8SurfaceIndex = inParam.cmSurfIndexHandle;

     return CM_SUCCESS;
 }

 CM_RT_API int32_t CmDevice_RT::CreateSamplerSurface2DEx(CmSurface2D* surface2d, SurfaceIndex* & samplerSurface2dIndex, CM_FLAG* flag)
 {
     INSERT_PROFILER_RECORD();

     CmSurface2D* surface2dRT = static_cast< CmSurface2D* >(surface2d);
     if (!surface2dRT)  {
         CmAssert(0);
         return CM_INVALID_ARG_VALUE;
     }

     CM_CREATESAMPLER2DEX_PARAM inParam;
     CmSafeMemSet(&inParam, 0, sizeof(inParam));
     inParam.cmSurface2DHandle = surface2dRT;
     inParam.flag = flag;

     int32_t result
         = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D_EX,
                                &inParam, sizeof(inParam));

     CHK_FAILURE_RETURN(result);
     CHK_FAILURE_RETURN(inParam.returnValue);
     samplerSurface2dIndex = (SurfaceIndex*)inParam.samplerSurfIndexHandle;
     return CM_SUCCESS;
 }

CM_RT_API int32_t CmDevice_RT::DestroySampler8x8Surface(SurfaceIndex* &sampler8x8SurfaceIndex)
{
    INSERT_PROFILER_RECORD();

    CM_DESTROYSAMPLER8x8SURF_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYSAMPLER8x8SURF_PARAM ) );
    inParam.cmSurfIndexHandle = sampler8x8SurfaceIndex;

    int32_t result
        = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYSAMPLER8X8SURFACE,
                               &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    sampler8x8SurfaceIndex = nullptr;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::SetL3Config(const L3ConfigRegisterValues *registerValues)
{
    INSERT_PROFILER_RECORD();

    m_l3Config = *registerValues;

    SetCapsInternal(CAP_L3_CONFIG, sizeof(L3ConfigRegisterValues), &m_l3Config);

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::SetSuggestedL3Config( L3_SUGGEST_CONFIG configIndex)
{
    INSERT_PROFILER_RECORD();

    //Call into UMD
    CM_DEVICE_SETSUGGESTEDL3_PARAM setL3IndexParam;
    CmSafeMemSet(&setL3IndexParam, 0 , sizeof(CM_DEVICE_SETSUGGESTEDL3_PARAM));
    setL3IndexParam.l3SuggestConfig = configIndex;

    int32_t result = OSALExtensionExecute(CM_FN_CMDEVICE_SETSUGGESTEDL3CONFIG,
                                          &setL3IndexParam,
                                          sizeof(setL3IndexParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(setL3IndexParam.returnValue);
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::SetCaps(CM_DEVICE_CAP_NAME capName, size_t capValueSize, void* capValue )
{
    INSERT_PROFILER_RECORD();

    switch(capName)
    {
    case CAP_HW_THREAD_COUNT:
        return SetCapsInternal(capName, capValueSize, capValue);

    default:
        return CM_INVALID_CAP_NAME;
    }
}

int32_t CmDevice_RT::SetCapsInternal(CM_DEVICE_CAP_NAME capName, size_t capValueSize, void* capValue)
{

    //Call into UMD
    CM_DEVICE_SETCAP_PARAM setCapParam;
    CmSafeMemSet(&setCapParam, 0 , sizeof(setCapParam));
    setCapParam.capName = capName;
    setCapParam.capValueSize = capValueSize;
    setCapParam.capValue = capValue;

    int32_t result = OSALExtensionExecute(CM_FN_CMDEVICE_SETCAPS,
                                          &setCapParam, sizeof(setCapParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(setCapParam.returnValue);
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateSamplerSurface2D(CmSurface2D* surface2d, SurfaceIndex* & samplerSurface2dIndex)
{
    INSERT_PROFILER_RECORD();

    CmSurface2D* surface2dRT = static_cast< CmSurface2D* >( surface2d );
    if( ! surface2dRT )  {
        CmAssert( 0 );
        return CM_FAILURE;
    }

    CM_CREATESAMPLER2D_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATESAMPLER2D_PARAM ) );
    inParam.cmSurface2DHandle  = surface2dRT;

    int32_t result
        = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D,
                               &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    samplerSurface2dIndex = (SurfaceIndex* )inParam.samplerSurfIndexHandle;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateSamplerSurface2DUP(CmSurface2DUP* surface2dUP, SurfaceIndex* & samplerSurface2dUPIndex)
{
    INSERT_PROFILER_RECORD();

    CM_CREATESAMPLER2DUP_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATESAMPLER2DUP_PARAM ) );
    inParam.cmSurface2DHandle = surface2dUP;

    int32_t result
        = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESAMPLERSURFACE2DUP,
                               &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    samplerSurface2dUPIndex = (SurfaceIndex* )inParam.samplerSurfIndexHandle;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateSamplerSurface3D(CmSurface3D* surface3d, SurfaceIndex* & samplerSurface3dIndex)
{
    INSERT_PROFILER_RECORD();

    CM_CREATESAMPLER3D_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATESAMPLER3D_PARAM ) );
    inParam.cmSurface3DHandle  = surface3d;

    int32_t result
        = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESAMPLERSURFACE3D,
                               &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);
    samplerSurface3dIndex = (SurfaceIndex* )inParam.samplerSurfIndexHandle;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::DestroySamplerSurface(SurfaceIndex* & samplerSurfaceIndex)
{
    INSERT_PROFILER_RECORD();

    CM_DESTROYSAMPLERSURF_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYSAMPLERSURF_PARAM ) );
    inParam.samplerSurfIndexHandle = samplerSurfaceIndex;

    int32_t result = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYSAMPLERSURFACE,
                                          &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT:: DestroySurface( CmSurface3D* &surface3d)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->DestroySurface3D(surface3d);
}

int32_t CmDevice_RT::CheckDdiVersionSupported(const uint32_t ddiVersion)
{
    if( ( ddiVersion >= CM_DDI_7_2 ))
    {
        return CM_SUCCESS;
    }
    else
    {
        return CM_UMD_DRIVER_NOT_SUPPORTED;
    }
}

CM_RT_API int32_t CmDevice_RT::InitPrintBuffer(size_t size)
{
    INSERT_PROFILER_RECORD();

    CM_DEVICE_INIT_PRINT_BUFFER_PARAM initPrintBufferParam;
    CmSafeMemSet(&initPrintBufferParam, 0, sizeof(CM_DEVICE_INIT_PRINT_BUFFER_PARAM));
    initPrintBufferParam.printBufferSize = (uint32_t)size;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_INIT_PRINT_BUFFER,
                                      &initPrintBufferParam,
                                      sizeof(initPrintBufferParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(initPrintBufferParam.returnValue);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Flush the print buffer and dump it on the file
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
CM_RT_API  int32_t CmDevice_RT::FlushPrintBufferIntoFile(const char *filename)
{
    INSERT_PROFILER_RECORD();

    return FlushPrintBufferInternal(filename);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Flush the print buffer and dump it on stdout
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDevice_RT::FlushPrintBuffer()
{
    INSERT_PROFILER_RECORD();

    return FlushPrintBufferInternal(nullptr);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Internal function to flush print buffer on stdout or file.
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
 int32_t CmDevice_RT::FlushPrintBufferInternal(const char *filename)
{
    INSERT_PROFILER_RECORD();

    CM_DEVICE_FLUSH_PRINT_BUFFER_PARAM flushPrintBufferParam;
    CmSafeMemSet(&flushPrintBufferParam, 0, sizeof(CM_DEVICE_FLUSH_PRINT_BUFFER_PARAM));
    flushPrintBufferParam.fileName = filename;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_FLUSH_PRINT_BUFFER,
                                      &flushPrintBufferParam,
                                      sizeof(flushPrintBufferParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(flushPrintBufferParam.returnValue);

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateVebox( CmVebox* & vebox )
{
    INSERT_PROFILER_RECORD();

    CM_CREATEVEBOX_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATEVEBOX_PARAM ) );

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_CREATEVEBOX,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    vebox = (CmVebox *)inParam.cmVeboxHandle;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::DestroyVebox( CmVebox* & vebox )
{
    INSERT_PROFILER_RECORD();

    CM_DESTROYVEBOX_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( inParam ) );
    inParam.cmVeboxHandle = vebox;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYVEBOX,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    vebox = nullptr;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateBufferSVM(uint32_t size, void* &sysMem, uint32_t accessFlag, CmBufferSVM* & buffer)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateBufferSVM(size, sysMem, accessFlag, buffer);
}

CM_RT_API int32_t CmDevice_RT::DestroyBufferSVM( CmBufferSVM* &buffer)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->DestroyBufferSVM(buffer);
}

CM_RT_API int32_t CmDevice_RT::CreateBufferStateless(size_t size,
                                                     uint32_t option,
                                                     void *sysMem,
                                                     CmBufferStateless *&buffer)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateBufferStateless(size, option, sysMem, buffer);
}

CM_RT_API int32_t CmDevice_RT::DestroyBufferStateless(CmBufferStateless* &buffer)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->DestroyBufferStateless(buffer);
}

CM_RT_API int32_t CmDevice_RT::CreateSurface2DAlias(CmSurface2D* originalSurface, SurfaceIndex* &aliasIndex)
{
    INSERT_PROFILER_RECORD();

    CM_DEVICE_CREATE_SURF2D_ALIAS_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof(inParam) );

    inParam.cmSurface2DHandle  = originalSurface;
    inParam.surfaceIndexHandle = aliasIndex;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_CREATESURFACE2D_ALIAS,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    aliasIndex = (SurfaceIndex*)inParam.surfaceIndexHandle;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateBufferAlias(CmBuffer *originalBuffer, SurfaceIndex* &aliasIndex)
{
    INSERT_PROFILER_RECORD();

    CM_DEVICE_CREATE_BUFFER_ALIAS_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof(inParam) );

    inParam.cmBufferHandle = originalBuffer;
    inParam.surfaceIndexHandle = aliasIndex;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_CREATEBUFFER_ALIAS,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    aliasIndex = (SurfaceIndex*)inParam.surfaceIndexHandle;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Duplicate the kernel member values
//| Arguments :
//|             destKernel     [in/out]    pointer to output kernel, must be nullptr
//|             srcKernel      [in]        pointer to input kernel
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDevice_RT::CloneKernel( CmKernel * &destKernel, CmKernel *srcKernel )
{
    INSERT_PROFILER_RECORD();

    CM_CLONE_KERNEL_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CLONE_KERNEL_PARAM ) );
    inParam.cmKernelHandleSrc  = srcKernel;
    inParam.cmKernelHandleDest = destKernel;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_CLONEKERNEL,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    destKernel = (CmKernel *)inParam.cmKernelHandleDest;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::CreateHevcVmeSurfaceG10(CmSurface2D* currentSurface, CmSurface2D** forwardSurfaceArray, CmSurface2D** backwardSurfaceArray, const uint32_t surfaceCountForward, const uint32_t surfaceCountBackward, SurfaceIndex* & vmeSurfaceIndex)
{
    INSERT_PROFILER_RECORD();

    return CreateVmeSurface( currentSurface, forwardSurfaceArray, backwardSurfaceArray, surfaceCountForward, surfaceCountBackward, vmeSurfaceIndex, CM_FN_CMDEVICE_CREATEHEVCVMESURFACEG10 );
}

CM_RT_API int32_t CmDevice_RT::DestroyHevcVmeSurfaceG10(SurfaceIndex* & vmeSurfaceIndex)
{
    INSERT_PROFILER_RECORD();

    return DestroyVmeSurface( vmeSurfaceIndex );
}

int32_t CmDevice_RT::CreateVmeSurface( CmSurface2D* currentSurface, CmSurface2D** forwardSurfaceArray, CmSurface2D** backwardSurfaceArray, const uint32_t surfaceCountForward, const uint32_t surfaceCountBackward, SurfaceIndex* & vmeSurfaceIndex, CM_FUNCTION_ID functionName)
{
    if ( currentSurface == nullptr )
    {
        CmAssert( 0 );
        return CM_NULL_POINTER;
    }

    CM_CREATEVMESURFACE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_CREATEVMESURFACE_PARAM ) );
    inParam.cmCurSurfHandle = currentSurface;
    inParam.cmForwardSurfArray = forwardSurfaceArray;
    inParam.cmBackwardSurfArray = backwardSurfaceArray;
    inParam.forwardSurfCount = surfaceCountForward;
    inParam.backwardSurfCount = surfaceCountBackward;

    int32_t result = OSALExtensionExecute(functionName,
                                          &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(inParam.returnValue);

    vmeSurfaceIndex = ( SurfaceIndex* )inParam.cmVmeSurfIndexHandle;

    return CM_SUCCESS;
}

int32_t CmDevice_RT::DestroyVmeSurface( SurfaceIndex* & vmeSurfaceIndex )
{
    //Call into driver
    CM_DESTROYVMESURFACE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_DESTROYVMESURFACE_PARAM ) );
    inParam.cmVmeSurfIndexHandle = ( void  *)vmeSurfaceIndex;

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYVMESURFACE,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    vmeSurfaceIndex = nullptr;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDevice_RT::GetVISAVersion(uint32_t& majorVersion, uint32_t& minorVersion)
{
    INSERT_PROFILER_RECORD();

    CM_DEVICE_GET_VISA_VERSION_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_DEVICE_GET_VISA_VERSION_PARAM));

    int32_t hr = OSALExtensionExecute(CM_FN_CMDEVICE_GETVISAVERSION,
                                      &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    majorVersion = inParam.majorVersion;
    minorVersion = inParam.minorVersion;
    return CM_SUCCESS;
}


CM_RT_API int32_t
CmDevice_RT::CreateSurface2DStateless(uint32_t width,
                                      uint32_t height,
                                      uint32_t &pitch,
                                      CmSurface2DStateless *&pSurface)
{
    return CM_NOT_IMPLEMENTED;
}

CM_RT_API int32_t CmDevice_RT::DestroySurface2DStateless(CmSurface2DStateless *&pSurface)
{
    return CM_NOT_IMPLEMENTED;
}
