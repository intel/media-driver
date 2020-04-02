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
//! \file  cm_global_api.cpp
//! \brief Contains implementations of gloabl CM APIs which are OS-agnostic.
//!

#include "cm_def.h"
#include "cm_buffer.h"
#include "cm_surface_2d.h"
#include "cm_surface_3d.h"
#include "cm_kernel.h"
#include "cm_task.h"
#include "cm_event.h"
#include "cm_debug.h"

//!
//! \brief      Returns the corresponding CM_RETURN_CODE error string.
//! \param      [in] errCode
//!             CM error code.
//! \return     Corresponding error string if valid Code. \n
//!             "Internal Error" if invalid.
//!
CM_RT_API const char* GetCmErrorString(int errCode)
{
    if (errCode == CM_SUCCESS)
    {
        return nullptr;
    }

    static const char *errorStrings[] = {
#define ENUM_STRING(e)  #e
        ENUM_STRING(CM_SUCCESS),
        ENUM_STRING(CM_FAILURE),
        ENUM_STRING(CM_NOT_IMPLEMENTED),
        ENUM_STRING(CM_SURFACE_ALLOCATION_FAILURE),
        ENUM_STRING(CM_OUT_OF_HOST_MEMORY),
        ENUM_STRING(CM_SURFACE_FORMAT_NOT_SUPPORTED),
        ENUM_STRING(CM_EXCEED_SURFACE_AMOUNT),
        ENUM_STRING(CM_EXCEED_KERNEL_ARG_AMOUNT),
        ENUM_STRING(CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE),
        ENUM_STRING(CM_INVALID_ARG_INDEX),
        ENUM_STRING(CM_INVALID_ARG_VALUE),
        ENUM_STRING(CM_INVALID_ARG_SIZE),
        ENUM_STRING(CM_INVALID_THREAD_INDEX),
        ENUM_STRING(CM_INVALID_WIDTH),
        ENUM_STRING(CM_INVALID_HEIGHT),
        ENUM_STRING(CM_INVALID_DEPTH),
        ENUM_STRING(CM_INVALID_COMMON_ISA),
        ENUM_STRING(CM_OPEN_VIDEO_DEVICE_HANDLE_FAILURE),
        ENUM_STRING( CM_VIDEO_DEVICE_LOCKED),
        ENUM_STRING(CM_LOCK_VIDEO_DEVICE_FAILURE),
        ENUM_STRING(CM_EXCEED_SAMPLER_AMOUNT),
        ENUM_STRING(CM_EXCEED_MAX_KERNEL_PER_ENQUEUE),
        ENUM_STRING(CM_EXCEED_MAX_KERNEL_SIZE_IN_BYTE),
        ENUM_STRING(CM_EXCEED_MAX_THREAD_AMOUNT_PER_ENQUEUE),
        ENUM_STRING(CM_EXCEED_VME_STATE_G6_AMOUNT),
        ENUM_STRING(CM_INVALID_THREAD_SPACE),
        ENUM_STRING(CM_EXCEED_MAX_TIMEOUT),
        ENUM_STRING(CM_JITDLL_LOAD_FAILURE),
        ENUM_STRING(CM_JIT_COMPILE_FAILURE),
        ENUM_STRING(CM_JIT_COMPILESIM_FAILURE),
        ENUM_STRING(CM_INVALID_THREAD_GROUP_SPACE),
        ENUM_STRING(CM_THREAD_ARG_NOT_ALLOWED),
        ENUM_STRING(CM_INVALID_GLOBAL_BUFFER_INDEX),
        ENUM_STRING(CM_INVALID_BUFFER_HANDLER),
        ENUM_STRING(CM_EXCEED_MAX_SLM_SIZE),
        ENUM_STRING(CM_JITDLL_OLDER_THAN_ISA),
        ENUM_STRING(CM_INVALID_HARDWARE_THREAD_NUMBER),
        ENUM_STRING(CM_GTPIN_INVOKE_FAILURE),
        ENUM_STRING(CM_INVALIDE_L3_CONFIGURATION),
        ENUM_STRING(CM_INVALID_TEXTURE2D_USAGE),
        ENUM_STRING(CM_INTEL_GFX_NOTFOUND),
        ENUM_STRING(CM_GPUCOPY_INVALID_SYSMEM),
        ENUM_STRING(CM_GPUCOPY_INVALID_WIDTH),
        ENUM_STRING(CM_GPUCOPY_INVALID_STRIDE),
        ENUM_STRING(CM_EVENT_DRIVEN_FAILURE),
        ENUM_STRING(CM_LOCK_SURFACE_FAIL),
        ENUM_STRING(CM_INVALID_GENX_BINARY),
        ENUM_STRING(CM_FEATURE_NOT_SUPPORTED_IN_DRIVER),
        ENUM_STRING(CM_QUERY_DLL_VERSION_FAILURE),
        ENUM_STRING(CM_KERNELPAYLOAD_PERTHREADARG_MUTEX_FAIL),
        ENUM_STRING(CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL),
        ENUM_STRING(CM_KERNELPAYLOAD_SETTING_FAILURE),
        ENUM_STRING(CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX),
        ENUM_STRING(CM_NOT_SET_KERNEL_ARGUMENT),
        ENUM_STRING(CM_GPUCOPY_INVALID_SURFACES),
        ENUM_STRING(CM_GPUCOPY_INVALID_SIZE),
        ENUM_STRING(CM_GPUCOPY_OUT_OF_RESOURCE),
        ENUM_STRING(CM_INVALID_VIDEO_DEVICE),
        ENUM_STRING(CM_SURFACE_DELAY_DESTROY),
        ENUM_STRING(CM_INVALID_VEBOX_STATE),
        ENUM_STRING(CM_INVALID_VEBOX_SURFACE),
        ENUM_STRING(CM_FEATURE_NOT_SUPPORTED_BY_HARDWARE),
        ENUM_STRING(CM_RESOURCE_USAGE_NOT_SUPPORT_READWRITE),
        ENUM_STRING(CM_MULTIPLE_MIPLEVELS_NOT_SUPPORTED),
        ENUM_STRING(CM_INVALID_UMD_CONTEXT),
        ENUM_STRING(CM_INVALID_LIBVA_SURFACE),
        ENUM_STRING(CM_INVALID_LIBVA_INITIALIZE),
        ENUM_STRING(CM_KERNEL_THREADSPACE_NOT_SET),
        ENUM_STRING(CM_INVALID_KERNEL_THREADSPACE),
        ENUM_STRING(CM_KERNEL_THREADSPACE_THREADS_NOT_ASSOCIATED),
        ENUM_STRING(CM_KERNEL_THREADSPACE_INTEGRITY_FAILED),
        ENUM_STRING(CM_INVALID_USERPROVIDED_GENBINARY),
        ENUM_STRING(CM_INVALID_PRIVATE_DATA),
        ENUM_STRING(CM_INVALID_MOS_RESOURCE_HANDLE),
        ENUM_STRING(CM_SURFACE_CACHED),
        ENUM_STRING(CM_SURFACE_IN_USE),
        ENUM_STRING(CM_INVALID_GPUCOPY_KERNEL),
        ENUM_STRING(CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN),
        ENUM_STRING(CM_INVALID_MEDIA_WALKING_PATTERN),
        ENUM_STRING(CM_FAILED_TO_ALLOCATE_SVM_BUFFER),
        ENUM_STRING(CM_EXCEED_MAX_POWER_OPTION_FOR_PLATFORM),
        ENUM_STRING(CM_INVALID_KERNEL_THREADGROUPSPACE),
        ENUM_STRING(CM_INVALID_KERNEL_SPILL_CODE),
        ENUM_STRING(CM_UMD_DRIVER_NOT_SUPPORTED),
        ENUM_STRING(CM_INVALID_GPU_FREQUENCY_VALUE),
        ENUM_STRING(CM_SYSTEM_MEMORY_NOT_4KPAGE_ALIGNED),
        ENUM_STRING(CM_KERNEL_ARG_SETTING_FAILED),
        ENUM_STRING(CM_NO_AVAILABLE_SURFACE),
        ENUM_STRING(CM_VA_SURFACE_NOT_SUPPORTED),
        ENUM_STRING(CM_TOO_MUCH_THREADS),
        ENUM_STRING(CM_NULL_POINTER),
        ENUM_STRING(CM_EXCEED_MAX_NUM_2D_ALIASES),
        ENUM_STRING(CM_INVALID_PARAM_SIZE),
        ENUM_STRING(CM_GT_UNSUPPORTED),
        ENUM_STRING(CM_GTPIN_FLAG_NO_LONGER_SUPPORTED),
        ENUM_STRING(CM_PLATFORM_UNSUPPORTED_FOR_API),
        ENUM_STRING(CM_TASK_MEDIA_RESET),
        ENUM_STRING(CM_KERNELPAYLOAD_SAMPLER_INVALID_BTINDEX),
        ENUM_STRING(CM_EXCEED_MAX_NUM_BUFFER_ALIASES),
        ENUM_STRING(CM_SYSTEM_MEMORY_NOT_4PIXELS_ALIGNED),
        ENUM_STRING(CM_FAILED_TO_CREATE_CURBE_SURFACE),
        ENUM_STRING(CM_INVALID_CAP_NAME),
        ENUM_STRING(CM_INVALID_USER_GPU_CONTEXT_FOR_QUEUE_EX),
        ENUM_STRING(CM_INVALID_CREATE_OPTION_FOR_BUFFER_STATELESS),
        ENUM_STRING(CM_INVALID_KERNEL_ARG_POINTER),
#undef ENUM_STRING
    };

    const char *errorString = "Internal Error";
    if ((errCode >= CM_INVALID_KERNEL_ARG_POINTER) && (errCode <= CM_SUCCESS))
    {
        errorString = errorStrings[MOS_ABS(errCode)];
    }

    return errorString;
}

using namespace CMRT_UMD;
// Surface::GetIndex
EXTERN_C CM_DRIVER_EXPOSED int CMRT_GetBufferIndex(CmBuffer* pSurface, SurfaceIndex*& pIndex)
{
    CM_CHK_NULL_RETURN_CMERROR(pSurface);
    return pSurface->GetIndex(pIndex);
}
EXTERN_C CM_DRIVER_EXPOSED int CMRT_GetSurface2DIndex(CmSurface2D* pSurface, SurfaceIndex*& pIndex)
{
    CM_CHK_NULL_RETURN_CMERROR(pSurface);
    return pSurface->GetIndex(pIndex);
}
EXTERN_C CM_DRIVER_EXPOSED int CMRT_GetSurface3DIndex(CmSurface3D* pSurface, SurfaceIndex*& pIndex)
{
    CM_CHK_NULL_RETURN_CMERROR(pSurface);
    return pSurface->GetIndex(pIndex);
}

/// surface read API
EXTERN_C CM_DRIVER_EXPOSED int CMRT_ReadBuffer(CmBuffer* pBuffer, unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL)
{
    CM_CHK_NULL_RETURN_CMERROR(pBuffer);
    return pBuffer->ReadSurface(pSysMem, pEvent, sysMemSize);
}
EXTERN_C CM_DRIVER_EXPOSED int CMRT_ReadSurface2D(CmSurface2D* pSurface, unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL)
{
    CM_CHK_NULL_RETURN_CMERROR(pSurface);
    return pSurface->ReadSurface(pSysMem, pEvent, sysMemSize);
}
EXTERN_C CM_DRIVER_EXPOSED int CMRT_ReadSurface3D(CmSurface3D* pSurface, unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL)
{
    CM_CHK_NULL_RETURN_CMERROR(pSurface);
    return pSurface->ReadSurface(pSysMem, pEvent, sysMemSize);
}
/// surface write API
EXTERN_C CM_DRIVER_EXPOSED int CMRT_WriteBuffer(CmBuffer* pBuffer, const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL)
{
    CM_CHK_NULL_RETURN_CMERROR(pBuffer);
    return pBuffer->WriteSurface(pSysMem, pEvent, sysMemSize);
}
EXTERN_C CM_DRIVER_EXPOSED int CMRT_WriteSurface2D(CmSurface2D* pSurface, unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL)
{
    CM_CHK_NULL_RETURN_CMERROR(pSurface);
    return pSurface->WriteSurface(pSysMem, pEvent, sysMemSize);
}
EXTERN_C CM_DRIVER_EXPOSED int CMRT_WriteSurface3D(CmSurface2D* pSurface, unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL)
{
    CM_CHK_NULL_RETURN_CMERROR(pSurface);
    return pSurface->WriteSurface(pSysMem, pEvent, sysMemSize);
}

EXTERN_C CM_DRIVER_EXPOSED int CMRT_SetKernelArgument(CmKernel* pKernel, uint32_t index, size_t size, const void *pValue)
{
    CM_CHK_NULL_RETURN_CMERROR(pKernel);
    return pKernel->SetKernelArg(index, size, pValue);
}

EXTERN_C CM_DRIVER_EXPOSED int CMRT_AddKernel(CmTask* pTask, CmKernel* pKernel)
{
    CM_CHK_NULL_RETURN_CMERROR(pTask);
    return pTask->AddKernel(pKernel);
}

// sync
EXTERN_C CM_DRIVER_EXPOSED int CMRT_WaitForTaskFinished(CmEvent* pEvent, uint32_t TimeOutMs = 2000UL)
{
    CM_CHK_NULL_RETURN_CMERROR(pEvent);
    return pEvent->WaitForTaskFinished(TimeOutMs);
}

