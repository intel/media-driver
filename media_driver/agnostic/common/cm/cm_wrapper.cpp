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
//! \file      cm_wrapper.cpp
//! \brief     Contains implementations of OS-agnostic functions for executing
//!            commands from cmrtlib.
//!

#include "cm_wrapper.h"

#include "cm_device_rt.h"
#include "cm_hal.h"
#include "cm_kernel_rt.h"
#include "cm_program.h"
#include "cm_task_rt.h"
#include "cm_thread_space_rt.h"
#include "cm_group_space.h"
#include "cm_queue_rt.h"
#include "cm_sampler.h"
#include "cm_sampler8x8.h"
#include "cm_surface_2d_rt.h"
#include "cm_vebox_rt.h"
#include "cm_extension_creator.h"

#if USE_EXTENSION_CODE
#include "cm_gtpin.h"
#endif

using CMRT_UMD::CmWrapperEx;
static bool cmWrapperExRegistered = CmExtensionCreator<CmWrapperEx>::RegisterClass<CmWrapperEx>();

struct CM_ENQUEUE_GPUCOPY_PARAM
{
    void *queueHandle;  // [in] CmQueue pointer in CMRT@UMD
    void *surface2d;    // [in] CmSurface2d pointer in CMRT@UMD
    void *sysMem;         // [in] pointer of system memory
    CM_GPUCOPY_DIRECTION copyDir;  // [in] direction for GPUCopy: CM_FASTCOPY_GPU2CPU (0) or CM_FASTCOPY_CPU2GPU(1)
    uint32_t widthStride;   // [in] width stride in byte for system memory, ZERO means no setting
    uint32_t heightStride;  // [in] height stride in row for system memory, ZERO means no setting
    uint32_t option;        // [in] option passed by user, only support CM_FASTCOPY_OPTION_NONBLOCKING(0) and CM_FASTCOPY_OPTION_BLOCKING(1)
    void *eventHandle;    // [in/out] return CmDevice pointer in CMRT@UMD, nullptr if the input is CM_NO_EVENT
    uint32_t eventIndex;    // [out] index of Event in m_EventArray
    int32_t returnValue;    // [out] return value from CMRT@UMD
};

struct CM_CREATEBUFFER_PARAM
{
    size_t size;             // [in]  buffer size in byte
    CM_BUFFER_TYPE bufferType;  // [in]  Buffer type (Buffer, BufferUP, or Buffer SVM)
    void *sysMem;              // [in]  Address of system memory
    void *bufferHandle;      // [out] pointer to CmBuffer object in CMRT@UMD
    int32_t returnValue;       // [out] the return value from CMRT@UMD
    uint32_t option;
};

namespace CMRT_UMD
{
void CmWrapperEx::Initialize(void *context)
{
    UNUSED(context);
}

int CmWrapperEx::Execute(
            CmDevice *device,
            CM_FUNCTION_ID cmFunctionID,
            void *inputData,
            uint32_t inputDataLen)
{
    UNUSED(device);
    UNUSED(cmFunctionID);
    UNUSED(inputData);
    UNUSED(inputDataLen);

    // currently, there is no extended functionality in this class
    // This interface is for future usage
    CM_ASSERTMESSAGE("Error: Invalid Function code '0x%x'.", cmFunctionID);
    // since there is no extended functionality now, it should return invalid function ID because
    // the function ID is not supported for now
    return CM_INVALID_PRIVATE_DATA;
}
};

//*-----------------------------------------------------------------------------
//| Purpose:    Get Time in ms and used in performance measurement
//| Returns:    Current system time.
//*-----------------------------------------------------------------------------
double CmGetTimeInms()
{
    LARGE_INTEGER   freq;
    LARGE_INTEGER   count;
    double   curTime;

    MOS_QueryPerformanceFrequency((uint64_t*)&freq.QuadPart);
    MOS_QueryPerformanceCounter((uint64_t*)&count.QuadPart);
    curTime = (double)(1000.0 * count.QuadPart/(double)freq.QuadPart);

    return curTime;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Convert GMM_RESROUCE_Format got by GetCaps to OSAL format
//| Return:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t ConvertToOperatingSystemAbstractionLayerFormat(void *src,
                                                       uint32_t numOfFormats)
{
    uint32_t i = 0 ;
    if(src == nullptr || numOfFormats == 0)
    {
        CM_ASSERTMESSAGE("Error: Invalid input arguments.");
        return CM_INVALID_ARG_VALUE;
    }

    for ( i=0 ; i< numOfFormats ; i++)
    {
        *((CM_OSAL_SURFACE_FORMAT *)src+i) = CmMosFmtToOSFmt(*((CM_SURFACE_FORMAT *)src+i));
    }
    return CM_SUCCESS;
}

#define CM_SET_NOFAILURE_STATUS(hr) \
        if (hr != CM_FAILURE) \
        { \
            hr = CM_SUCCESS; \
        }

#if USE_EXTENSION_CODE
extern int CmThinExecuteEnableGTPin(CmDevice *device, void *cmPrivateInputData);
extern int CmThinExecuteRegGTPinMarkers(CmDevice *device, void *cmPrivateInputData);
#endif

using CMRT_UMD::CmSurface2D;
using CMRT_UMD::CmSurface2DRT;
using CMRT_UMD::SurfaceIndex;
using CMRT_UMD::CmDeviceRT;
using CMRT_UMD::CmDeviceRTBase;
using CMRT_UMD::CmBuffer;
using CMRT_UMD::CmBufferUP;
using CMRT_UMD::CmBufferSVM;
using CMRT_UMD::CmSurface2DUP;
using CMRT_UMD::CmKernel;
using CMRT_UMD::CmKernelRT;
using CMRT_UMD::CmProgram;
using CMRT_UMD::CmProgramRT;
using CMRT_UMD::CmTask;
using CMRT_UMD::CmTaskRT;
using CMRT_UMD::CmQueue;
using CMRT_UMD::CmQueueRT;
using CMRT_UMD::CmThreadSpace;
using CMRT_UMD::CmThreadGroupSpace;
using CMRT_UMD::CmEvent;
using CMRT_UMD::CmThreadSpaceRT;
using CMRT_UMD::CmSampler;
using CMRT_UMD::CmVebox;
using CMRT_UMD::CmVeboxRT;
using CMRT_UMD::CmSurface3D;
using CMRT_UMD::CmSampler8x8;
using CMRT_UMD::CmBufferStateless;
//*-----------------------------------------------------------------------------
//| Purpose:    CMRT thin layer library supported function execution
//| Return:     CM_SUCCESS if successful
//*-----------------------------------------------------------------------------
int32_t CmThinExecuteInternal(CmDevice *device,
                        CM_FUNCTION_ID cmFunctionID,
                        void *cmPrivateInputData,
                        uint32_t cmPrivateInputDataSize)
{
    int32_t                     hr                 = CM_SUCCESS;
    int32_t                     cmRet              = CM_INVALID_PRIVATE_DATA;
    CmBuffer                    *cmBuffer          = nullptr;
    CmBufferUP                  *cmBufferUP        = nullptr;
    CmBufferSVM                 *cmBufferSVM       = nullptr;
    CmBufferStateless           *cmBufferStateless = nullptr;
    CmSurface2DRT               *cmSurface2d       = nullptr;
    CmSurface2DUP               *cmSurface2dup     = nullptr;
    CmProgram                   *cmProgram         = nullptr;
    CmKernel                    *cmKernel          = nullptr;
    CmTask                      *cmTask            = nullptr;
    CmQueue                     *cmQueue           = nullptr;
    CmQueueRT                   *cmQueueRT         = nullptr;
    CmEvent                     *cmEvent           = nullptr;
    CmThreadSpace               *cmThreadSpace     = nullptr;
    CmThreadGroupSpace          *cmThreadGroupSpace = nullptr;
    SurfaceIndex                *vmeSurIndex       = nullptr;
    SurfaceIndex                *surfaceIndex      = nullptr;
    CmSampler                   *sampler           = nullptr;
    CMRT_UMD::SamplerIndex      *samplerIndex      = nullptr;
    CmThreadGroupSpace          *threadGrpSpace    = nullptr;
    CmSurface3D                 *cmSurface3d       = nullptr;
    CmSampler8x8                *sampler8x8        = nullptr;
    CmSurface2D                 *cmSrcSurface2d    = nullptr;
    CmSurface2D                 *cmDstSurface2d    = nullptr;
    CmSurface2D                 *cmSurf2DBase      = nullptr;
    CmVebox                     *cmVebox           = nullptr;
    CmDeviceRT                  *deviceRT          = nullptr;
    CmDeviceRTBase              *deviceRtBase      = nullptr;

    if (cmPrivateInputData == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Null pointer.");
        return CM_INVALID_PRIVATE_DATA;
    }

    if ((cmFunctionID != CM_FN_RT_ULT_INFO && cmFunctionID != CM_FN_RT_ULT)&&(device == nullptr))
    {
        CM_ASSERTMESSAGE("Error: Null pointer.");
        return CM_NULL_POINTER;
    }
    deviceRT = static_cast<CmDeviceRT*>(device);
    deviceRtBase = static_cast<CmDeviceRTBase *>(device);

    switch(cmFunctionID)
    {
    case CM_FN_CMDEVICE_CREATEBUFFER:
        if(cmPrivateInputDataSize == sizeof(CM_CREATEBUFFER_PARAM))
        {
            CM_CREATEBUFFER_PARAM *pCmBufferParam
                = (CM_CREATEBUFFER_PARAM*)(cmPrivateInputData);

            if(pCmBufferParam->bufferType == CM_BUFFER_N)
            {
                //Create Buffer
                cmRet = device->CreateBuffer(pCmBufferParam->size, cmBuffer);
                //Create Surface Index
                if( cmRet == CM_SUCCESS)
                {
                    pCmBufferParam->bufferHandle = static_cast<CmBuffer *>(cmBuffer);
                }
                //Fill the output message
                pCmBufferParam->returnValue        = cmRet;
            }
            else if(pCmBufferParam->bufferType == CM_BUFFER_UP)
            { // Create Buffer Up
                cmRet = device->CreateBufferUP(pCmBufferParam->size,
                                             pCmBufferParam->sysMem,
                                             cmBufferUP);
                //Create Surface Index
                if( cmRet == CM_SUCCESS)
                {
                    pCmBufferParam->bufferHandle     = cmBufferUP;
                }
                //Fill the output message
                pCmBufferParam->returnValue        = cmRet;
            }
            else if (pCmBufferParam->bufferType == CM_BUFFER_SVM)
            {
                cmRet = device->CreateBufferSVM(pCmBufferParam->size,
                                                 pCmBufferParam->sysMem,
                                                 0,
                                                 cmBufferSVM);
                if (cmRet == CM_SUCCESS)
                {
                    pCmBufferParam->bufferHandle = cmBufferSVM;
                }
                //Fill the output message
                pCmBufferParam->returnValue        = cmRet;
            }
            else if (pCmBufferParam->bufferType == CM_BUFFER_STATELESS)
            {
                cmRet = deviceRtBase->CreateBufferStateless(pCmBufferParam->size,
                                                            pCmBufferParam->option,
                                                            pCmBufferParam->sysMem,
                                                            cmBufferStateless);
                if (cmRet == CM_SUCCESS)
                {
                    pCmBufferParam->bufferHandle = cmBufferStateless;
                }
                //Fill the output message
                pCmBufferParam->returnValue = cmRet;
            }
            else //should never jump here
            {
                pCmBufferParam->returnValue = CM_INVALID_ARG_VALUE;
                goto finish;
            }
        }
        else
        {
            hr = CM_INVALID_PRIVATE_DATA;
            goto finish;
        }
        break;

    case CM_FN_CMDEVICE_DESTROYBUFFER:
        PCM_DESTROYBUFFER_PARAM cmDestBufferParam;
        cmDestBufferParam = (PCM_DESTROYBUFFER_PARAM)(cmPrivateInputData);

        cmBuffer = (CmBuffer *)(cmDestBufferParam->bufferHandle);
        CM_ASSERT(cmBuffer);

        cmRet = device->DestroySurface(cmBuffer);

        //Fill the output message
        cmDestBufferParam->returnValue  = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYBUFFERUP:
        PCM_DESTROYBUFFER_PARAM cmDestBufferUPParam;
        cmDestBufferUPParam = (PCM_DESTROYBUFFER_PARAM)(cmPrivateInputData);

        cmBufferUP= (CmBufferUP *)(cmDestBufferUPParam->bufferHandle);
        CM_ASSERT(cmBufferUP);

        cmRet = device->DestroyBufferUP(cmBufferUP);

        //Fill the output message
        cmDestBufferUPParam->returnValue  = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYBUFFERSVM:
        PCM_DESTROYBUFFER_PARAM cmDestBufferSVMParam;
        cmDestBufferSVMParam = (PCM_DESTROYBUFFER_PARAM)(cmPrivateInputData);

        cmBufferSVM = (CmBufferSVM *)(cmDestBufferSVMParam->bufferHandle);
        CM_ASSERT(cmBufferSVM);

        cmRet = device->DestroyBufferSVM(cmBufferSVM);
        //Fill the output message
        cmDestBufferSVMParam->returnValue  = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYBUFFERSTATELESS:
        PCM_DESTROYBUFFER_PARAM cmDestBufferStatelessParam;
        cmDestBufferStatelessParam = (PCM_DESTROYBUFFER_PARAM)(cmPrivateInputData);

        cmBufferStateless =
            static_cast<CmBufferStateless *>(cmDestBufferStatelessParam->bufferHandle);
        CM_ASSERT(cmBufferStateless);

        cmRet = deviceRtBase->DestroyBufferStateless(cmBufferStateless);
        //Fill the output message
        cmDestBufferStatelessParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESURFACE2DUP:
        PCM_CREATESURFACE2DUP_PARAM cmCreate2DUpParam;
        cmCreate2DUpParam = (PCM_CREATESURFACE2DUP_PARAM)(cmPrivateInputData);

        cmRet = device->CreateSurface2DUP(cmCreate2DUpParam->width,
            cmCreate2DUpParam->height,
            CmOSFmtToMosFmt(cmCreate2DUpParam->format),
            cmCreate2DUpParam->sysMem,
            cmSurface2dup);
        if( cmRet == CM_SUCCESS)
        {
            cmCreate2DUpParam->surface2DUPHandle = static_cast<CmSurface2DUP *>(cmSurface2dup);
        }

        cmCreate2DUpParam->returnValue         = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSURFACE2DUP:
        PCM_DESTROYSURFACE2DUP_PARAM cmDestroy2DUpParam;
        cmDestroy2DUpParam = (PCM_DESTROYSURFACE2DUP_PARAM)(cmPrivateInputData);
        cmSurface2dup = static_cast<CmSurface2DUP *>(cmDestroy2DUpParam->surface2DUPHandle);

        cmRet = device->DestroySurface2DUP(cmSurface2dup);

        cmDestroy2DUpParam->returnValue        = cmRet;
        break;

    case CM_FN_CMDEVICE_GETSURFACE2DINFO:
        PCM_GETSURFACE2DINFO_PARAM cmGet2DinfoParam;
        uint32_t pitch,physicalsize;
        cmGet2DinfoParam = (PCM_GETSURFACE2DINFO_PARAM)(cmPrivateInputData);

        cmRet = device->GetSurface2DInfo( cmGet2DinfoParam->width,
                                        cmGet2DinfoParam->height,
                                        CmOSFmtToMosFmt(cmGet2DinfoParam->format),
                                        pitch,
                                        physicalsize);

        cmGet2DinfoParam->pitch           = pitch;
        cmGet2DinfoParam->physicalSize    = physicalsize;
        cmGet2DinfoParam->returnValue     = cmRet;
        break;

     case CM_FN_CMDEVICE_CREATESURFACE2D_ALIAS:
         {
             PCM_DEVICE_CREATE_SURF2D_ALIAS_PARAM createSurf2DAliasParam;
             createSurf2DAliasParam = (PCM_DEVICE_CREATE_SURF2D_ALIAS_PARAM)(cmPrivateInputData);
             CmSurface2D * cmSurfBase = static_cast<CmSurface2D *>(createSurf2DAliasParam->surface2DHandle);
             surfaceIndex = (SurfaceIndex*)createSurf2DAliasParam->surfaceIndexHandle;

             cmRet = device->CreateSurface2DAlias(cmSurfBase, surfaceIndex);
             createSurf2DAliasParam->surfaceIndexHandle = surfaceIndex;
             createSurf2DAliasParam->returnValue = cmRet;
         }
         break;

     case CM_FN_CMDEVICE_CREATEBUFFER_ALIAS:
         {
             PCM_DEVICE_CREATE_BUFFER_ALIAS_PARAM createBufferAliasParam;
             createBufferAliasParam = (PCM_DEVICE_CREATE_BUFFER_ALIAS_PARAM)(cmPrivateInputData);
             cmBuffer= static_cast<CmBuffer *>(createBufferAliasParam->bufferHandle);
             surfaceIndex = (SurfaceIndex*)createBufferAliasParam->surfaceIndexHandle;

             cmRet = device->CreateBufferAlias(cmBuffer, surfaceIndex);
             createBufferAliasParam->surfaceIndexHandle = surfaceIndex;
             createBufferAliasParam->returnValue = cmRet;
         }
         break;

     case CM_FN_CMDEVICE_CLONEKERNEL:
         {
             PCM_CLONE_KERNEL_PARAM  cloneKernelParam;
             cloneKernelParam = (PCM_CLONE_KERNEL_PARAM)(cmPrivateInputData);
             CmKernel * kernelDest = static_cast<CmKernel *>(cloneKernelParam->kernelHandleDest);
             CmKernel * kernelSrc = static_cast<CmKernel *>(cloneKernelParam->kernelHandleSrc);
             cmRet = device->CloneKernel(kernelDest, kernelSrc);
             cloneKernelParam->kernelHandleDest = kernelDest;

             cloneKernelParam->returnValue = cmRet;
         }
         break;

     case CM_FN_CMDEVICE_DESTROYSURFACE2D:
        {
            PCM_DESTROYSURFACE2D_PARAM cmDestroy2DParam;

            cmDestroy2DParam    = (PCM_DESTROYSURFACE2D_PARAM)(cmPrivateInputData);
            CmSurface2D * cmSurfBase = (CmSurface2D *)(cmDestroy2DParam->surface2DHandle);

            CM_ASSERT(cmSurfBase);

            cmRet = device->DestroySurface(cmSurfBase);
            //Fill output message
            cmDestroy2DParam->surface2DHandle    = nullptr;
            cmDestroy2DParam->returnValue        = cmRet;
        }
        break;

    case CM_FN_CMDEVICE_LOADPROGRAM:
        PCM_LOADPROGRAM_PARAM       cmLoadProgParam;
        cmLoadProgParam     = (PCM_LOADPROGRAM_PARAM)(cmPrivateInputData);
        CM_ASSERT(cmLoadProgParam->cisaCode);

        cmRet = device->LoadProgram(cmLoadProgParam->cisaCode,
                                     cmLoadProgParam->cisaCodeSize,
                                     cmProgram,
                                     cmLoadProgParam->options);

        if(cmRet == CM_SUCCESS && cmProgram)
        {
            cmLoadProgParam->programHandle  = static_cast<CmProgram *>(cmProgram);
            CmProgramRT *cmProgramRT = static_cast<CmProgramRT *>(cmProgram);
            cmLoadProgParam->indexInArray      = cmProgramRT->GetProgramIndex();
        }
        cmLoadProgParam->returnValue      = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYPROGRAM:
        PCM_DESTROYPROGRAM_PARAM   cmDestoyProgParam;
        cmDestoyProgParam = (PCM_DESTROYPROGRAM_PARAM)(cmPrivateInputData);
        cmProgram         = (CmProgram *)(cmDestoyProgParam->programHandle);

        CM_ASSERT(cmProgram);

        cmRet = device->DestroyProgram(cmProgram);

        cmDestoyProgParam->returnValue = cmRet;
        break;

     case CM_FN_CMDEVICE_CREATEKERNEL:
        PCM_CREATEKERNEL_PARAM  cmCreateKernelParam;
        cmCreateKernelParam =(PCM_CREATEKERNEL_PARAM)(cmPrivateInputData);
        cmProgram           = (CmProgram *)(cmCreateKernelParam->programHandle);

        cmRet = device->CreateKernel(cmProgram,
                                   cmCreateKernelParam->kernelName,
                                   cmKernel,
                                   cmCreateKernelParam->options);
        if(cmRet == CM_SUCCESS && cmKernel)
        {
            cmCreateKernelParam->kernelHandle = static_cast< CmKernel * >(cmKernel);
            CmKernelRT *kernelRT = static_cast<CmKernelRT *>(cmKernel);
            cmCreateKernelParam->indexKernelArray = kernelRT->GetKernelIndex();
        }
        cmCreateKernelParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYKERNEL:
        PCM_DESTROYKERNEL_PARAM cmDestroyKernelParam;
        cmDestroyKernelParam = (PCM_DESTROYKERNEL_PARAM)(cmPrivateInputData);
        cmKernel            =(CmKernel *)(cmDestroyKernelParam->kernelHandle);
        CM_ASSERT(cmKernel);

        cmRet = device->DestroyKernel(cmKernel);

        cmDestroyKernelParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATETASK:
        PCM_CREATETASK_PARAM cmCreateTaskParam;
        cmCreateTaskParam = (PCM_CREATETASK_PARAM)(cmPrivateInputData);
        cmRet = device->CreateTask(cmTask);

        if(cmRet == CM_SUCCESS && cmTask)
        {
            cmCreateTaskParam->taskHandle = cmTask;
            CmTaskRT *cmTaskRT = static_cast<CmTaskRT *>(cmTask);
            cmCreateTaskParam->taskIndex    = cmTaskRT->GetIndexInTaskArray();
        }
        cmCreateTaskParam->returnValue  = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYTASK:
        PCM_DESTROYTASK_PARAM cmDestroyTaskParam;
        cmDestroyTaskParam = (PCM_DESTROYTASK_PARAM)(cmPrivateInputData);
        cmTask             = (CmTask *)cmDestroyTaskParam->taskHandle;

        cmRet = device->DestroyTask(cmTask);

        cmDestroyTaskParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATEQUEUE:
    {
        CM_CREATEQUEUE_PARAM *cmCreateQueParam;
        cmCreateQueParam = (CM_CREATEQUEUE_PARAM *)(cmPrivateInputData);

        cmRet = device->CreateQueue(cmQueue);

        if (cmRet == CM_SUCCESS && cmQueue != nullptr)
        {
            // Sync queue option and handle to thin layer.
            CmQueueRT *cmQueueRT = static_cast<CmQueueRT *>(cmQueue);
            cmCreateQueParam->createOption  = cmQueueRT->GetQueueOption();
            cmCreateQueParam->queueHandle   = (cmQueue);
            cmCreateQueParam->returnValue   = CM_SUCCESS;
        }
        else
        {
            cmCreateQueParam->queueHandle   = nullptr;
            cmCreateQueParam->returnValue   = cmRet;
        }
        break;
    }

    case CM_FN_CMDEVICE_CREATEQUEUEEX:
    {
        CM_CREATEQUEUE_PARAM *cmCreateQueParam;
        cmCreateQueParam = (CM_CREATEQUEUE_PARAM *)(cmPrivateInputData);
        CM_ASSERT(cmCreateQueParam);

        cmRet = device->CreateQueueEx(cmQueue, cmCreateQueParam->createOption);

        cmCreateQueParam->returnValue = cmRet;
        cmCreateQueParam->queueHandle = (cmQueue);
        break;
    }

    case CM_FN_CMQUEUE_ENQUEUE:
    {
        PCM_ENQUEUE_PARAM cmEnqueueParam;
        cmEnqueueParam = (PCM_ENQUEUE_PARAM)(cmPrivateInputData);
        cmQueue        = (CmQueue *)cmEnqueueParam->queueHandle;
        cmTask         = (CmTask  *)cmEnqueueParam->taskHandle;
        cmThreadSpace           = (CmThreadSpace *)cmEnqueueParam->threadSpaceHandle;
        cmEvent        = (CmEvent*)cmEnqueueParam->eventHandle; // used as input

        CM_ASSERT(cmQueue);
        CM_ASSERT(cmTask);

        cmRet = cmQueue->Enqueue(cmTask,cmEvent,cmThreadSpace);

        cmEnqueueParam->eventHandle = cmEvent;
        cmEnqueueParam->returnValue = cmRet;
    }
        break;

     case CM_FN_CMQUEUE_ENQUEUEFAST:
     {
        PCM_ENQUEUE_PARAM cmEnqueueParam;
        cmEnqueueParam = (PCM_ENQUEUE_PARAM)(cmPrivateInputData);
        cmQueue        = (CmQueue *)cmEnqueueParam->queueHandle;
        cmTask         = (CmTask  *)cmEnqueueParam->taskHandle;
        cmThreadSpace           = (CmThreadSpace *)cmEnqueueParam->threadSpaceHandle;
        cmEvent        = (CmEvent*)cmEnqueueParam->eventHandle; // used as input

        CM_ASSERT(cmQueue);
        CM_ASSERT(cmTask);

        cmRet = cmQueue->EnqueueFast(cmTask,cmEvent,cmThreadSpace);

        cmEnqueueParam->eventHandle = cmEvent;
        cmEnqueueParam->returnValue = cmRet;
     }
        break;

     case CM_FN_CMQUEUE_ENQUEUEWITHHINTS:
        PCM_ENQUEUEHINTS_PARAM cmEnqueueHintsParam;
        cmEnqueueHintsParam = (PCM_ENQUEUEHINTS_PARAM)(cmPrivateInputData);
        cmQueue             = (CmQueue *)cmEnqueueHintsParam->queueHandle;
        cmTask              = (CmTask  *)cmEnqueueHintsParam->taskHandle;
        cmEvent             = (CmEvent *)cmEnqueueHintsParam->eventHandle; // used as input

        if(cmQueue)
        {
            cmRet = cmQueue->EnqueueWithHints(cmTask, cmEvent, cmEnqueueHintsParam->hints);
        }

        cmEnqueueHintsParam->eventHandle = cmEvent;
        cmEnqueueHintsParam->returnValue = cmRet;
        break;

    case CM_FN_CMQUEUE_DESTROYEVENT:
    {
        PCM_DESTROYEVENT_PARAM cmDestroyEventParam;
        cmDestroyEventParam = (PCM_DESTROYEVENT_PARAM)(cmPrivateInputData);
        cmQueue        = (CmQueue *)cmDestroyEventParam->queueHandle;
        cmEvent        = (CmEvent *)cmDestroyEventParam->eventHandle;
        CM_ASSERT(cmQueue);
        CM_ASSERT(cmEvent);
        
        cmRet = cmQueue->DestroyEvent(cmEvent);

        cmDestroyEventParam->returnValue = cmRet;
    }
        break;

    case CM_FN_CMQUEUE_DESTROYEVENTFAST:
    {
        PCM_DESTROYEVENT_PARAM cmDestroyEventParam;
        cmDestroyEventParam = (PCM_DESTROYEVENT_PARAM)(cmPrivateInputData);
        cmQueue        = (CmQueue *)cmDestroyEventParam->queueHandle;
        cmEvent        = (CmEvent *)cmDestroyEventParam->eventHandle;
        CM_ASSERT(cmQueue);
        CM_ASSERT(cmEvent);
        
        cmRet = cmQueue->DestroyEventFast(cmEvent);

        cmDestroyEventParam->returnValue = cmRet;
    }
        break;
        
    case CM_FN_CMDEVICE_CREATETHREADSPACE:
        PCM_CREATETHREADSPACE_PARAM cmCreateTsParam;
        cmCreateTsParam = (PCM_CREATETHREADSPACE_PARAM)(cmPrivateInputData);

        cmRet = device->CreateThreadSpace(cmCreateTsParam->threadSpaceWidth,
                                        cmCreateTsParam->threadSpaceHeight,
                                        cmThreadSpace);
        if(cmRet==CM_SUCCESS && cmThreadSpace)
        {
            cmCreateTsParam->threadSpaceHandle = cmThreadSpace;
            CmThreadSpaceRT *cmThreadSpaceRT = static_cast<CmThreadSpaceRT *>(cmThreadSpace);
            cmCreateTsParam->indexInTSArray = cmThreadSpaceRT->GetIndexInTsArray();
        }
        cmCreateTsParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYTHREADSPACE:
        PCM_DESTROYTHREADSPACE_PARAM cmDestroyTsParam;
        cmDestroyTsParam = (PCM_DESTROYTHREADSPACE_PARAM)(cmPrivateInputData);
        cmThreadSpace    = (CmThreadSpace *)cmDestroyTsParam->threadSpaceHandle;
        CM_ASSERT(cmThreadSpace);
        cmRet = device->DestroyThreadSpace(cmThreadSpace);
        cmDestroyTsParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATEVMESURFACEG7_5:
        PCM_CREATEVMESURFACE_PARAM  createVmeSurf7p5Param;
        createVmeSurf7p5Param = ( PCM_CREATEVMESURFACE_PARAM )( cmPrivateInputData );

        cmRet = device->CreateVmeSurfaceG7_5((CmSurface2D *) createVmeSurf7p5Param->curSurfHandle,
                                           (CmSurface2D * *) createVmeSurf7p5Param->forwardSurfArray,
                                           (CmSurface2D * * )createVmeSurf7p5Param->backwardSurfArray,
                                           createVmeSurf7p5Param->forwardSurfCount,
                                           createVmeSurf7p5Param->backwardSurfCount,
                                           vmeSurIndex);

        createVmeSurf7p5Param->vmeSurfIndexHandle = vmeSurIndex;
        createVmeSurf7p5Param->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATEHEVCVMESURFACEG10:
        PCM_CREATEVMESURFACE_PARAM  createVmeSurfParamG10;
        createVmeSurfParamG10 = ( PCM_CREATEVMESURFACE_PARAM )( cmPrivateInputData );

        cmRet = device->CreateHevcVmeSurfaceG10( ( CmSurface2D * )createVmeSurfParamG10->curSurfHandle,
                                             ( CmSurface2D * * )createVmeSurfParamG10->forwardSurfArray,
                                             ( CmSurface2D * * )createVmeSurfParamG10->backwardSurfArray,
                                             createVmeSurfParamG10->forwardSurfCount,
                                             createVmeSurfParamG10->backwardSurfCount,
                                             vmeSurIndex );

        createVmeSurfParamG10->vmeSurfIndexHandle = vmeSurIndex;
        createVmeSurfParamG10->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYVMESURFACE:
        PCM_DESTROYVMESURFACE_PARAM destroyVmeSurfParam;
        destroyVmeSurfParam = ( PCM_DESTROYVMESURFACE_PARAM )( cmPrivateInputData );

        vmeSurIndex = ( SurfaceIndex* )destroyVmeSurfParam->vmeSurfIndexHandle;
        cmRet = deviceRT->DestroyVmeSurface( vmeSurIndex );
        destroyVmeSurfParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CONFIGVMESURFACEDIMENSION:
        PCM_CONFIGVMESURFACEDIMENSION_PARAM configSurfStateParam;
        configSurfStateParam = (PCM_CONFIGVMESURFACEDIMENSION_PARAM)(cmPrivateInputData);

        vmeSurIndex = (SurfaceIndex* )configSurfStateParam->vmeSurfHandle;
        cmRet = device->SetVmeSurfaceStateParam(vmeSurIndex, configSurfStateParam->surfDimensionPara);
        configSurfStateParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESAMPLER:
        PCM_CREATESAMPLER_PARAM createSamplerParam;
        createSamplerParam = (PCM_CREATESAMPLER_PARAM)(cmPrivateInputData);

        cmRet = device->CreateSampler(createSamplerParam->sampleState, sampler);
        if(cmRet == CM_SUCCESS)
        {
             cmRet = sampler->GetIndex(samplerIndex);
             createSamplerParam->samplerHandle =  static_cast<CmSampler *>(sampler);
             createSamplerParam->samplerIndexHandle = samplerIndex;
        }
        createSamplerParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESAMPLER_EX:
        PCM_CREATESAMPLER_PARAM_EX createSamplerParamEx;
        createSamplerParamEx = (PCM_CREATESAMPLER_PARAM_EX)(cmPrivateInputData);

        cmRet = device->CreateSamplerEx(createSamplerParamEx->sampleState, sampler);
        if(cmRet == CM_SUCCESS)
        {
             cmRet = sampler->GetIndex(samplerIndex);
             createSamplerParamEx->samplerHandle =  static_cast<CmSampler *>(sampler);
             createSamplerParamEx->samplerIndexHandle = samplerIndex;
        }
        createSamplerParamEx->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSAMPLER:
        PCM_DESTROYSAMPLER_PARAM destroySamplerParam;
        destroySamplerParam = (PCM_DESTROYSAMPLER_PARAM)(cmPrivateInputData);

        sampler            = (CmSampler *)destroySamplerParam->samplerHandle;
        cmRet = device->DestroySampler(sampler);
        destroySamplerParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATETHREADGROUPSPACE:
        PCM_CREATETGROUPSPACE_PARAM createTGrpSpaceParam;
        createTGrpSpaceParam = (PCM_CREATETGROUPSPACE_PARAM)(cmPrivateInputData);

        cmRet  = device->CreateThreadGroupSpaceEx(
                 createTGrpSpaceParam->thrdSpaceWidth,
                 createTGrpSpaceParam->thrdSpaceHeight,
                 createTGrpSpaceParam->thrdSpaceDepth,
                 createTGrpSpaceParam->grpSpaceWidth,
                 createTGrpSpaceParam->grpSpaceHeight,
                 createTGrpSpaceParam->grpSpaceDepth,
                 threadGrpSpace);
        if(cmRet == CM_SUCCESS && threadGrpSpace)
        {
            createTGrpSpaceParam->groupSpaceHandle = static_cast<CmThreadGroupSpace *>(threadGrpSpace);
            createTGrpSpaceParam->threadGroupSpaceIndex = threadGrpSpace->GetIndexInTGsArray();
        }
        createTGrpSpaceParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYTHREADGROUPSPACE:
        PCM_DESTROYTGROPUSPACE_PARAM destroyTGrpSpaceParam;
        destroyTGrpSpaceParam = (PCM_DESTROYTGROPUSPACE_PARAM)(cmPrivateInputData);

        threadGrpSpace     = (CmThreadGroupSpace *)destroyTGrpSpaceParam->groupSpaceHandle;
        cmRet = device->DestroyThreadGroupSpace(threadGrpSpace);
        destroyTGrpSpaceParam->returnValue = cmRet;
        break;

    case CM_FN_CMQUEUE_ENQUEUEWITHGROUP:
    {
        PCM_ENQUEUEGROUP_PARAM enqueueGroupParam;
        enqueueGroupParam = (PCM_ENQUEUEGROUP_PARAM)(cmPrivateInputData);
        cmQueue = (CmQueue *)enqueueGroupParam->queueHandle;
        threadGrpSpace = (CmThreadGroupSpace *)enqueueGroupParam->threadGroupSpaceHandle;
        cmTask = (CmTask *)enqueueGroupParam->taskHandle;
        cmEvent = (CmEvent*)enqueueGroupParam->eventHandle; // used as input

        cmRet = cmQueue->EnqueueWithGroup(cmTask,
            cmEvent,
            threadGrpSpace);

        enqueueGroupParam->eventHandle = cmEvent;
        enqueueGroupParam->returnValue = cmRet;
    }
        break;

    case CM_FN_CMQUEUE_ENQUEUEWITHGROUPFAST:
    {
        PCM_ENQUEUEGROUP_PARAM enqueueGroupParam;
        enqueueGroupParam = (PCM_ENQUEUEGROUP_PARAM)(cmPrivateInputData);
        cmQueue = (CmQueue *)enqueueGroupParam->queueHandle;
        threadGrpSpace = (CmThreadGroupSpace *)enqueueGroupParam->threadGroupSpaceHandle;
        cmTask = (CmTask *)enqueueGroupParam->taskHandle;
        cmEvent = (CmEvent*)enqueueGroupParam->eventHandle; // used as input

        cmRet = cmQueue->EnqueueWithGroupFast(cmTask,
            cmEvent,
            threadGrpSpace);

        enqueueGroupParam->eventHandle = cmEvent;
        enqueueGroupParam->returnValue = cmRet;
    }
        break;

    case CM_FN_CMDEVICE_GETCAPS:
        PCM_GETCAPS_PARAM getCapParam;
        getCapParam = (PCM_GETCAPS_PARAM)(cmPrivateInputData);

        cmRet = device->GetCaps(getCapParam->capName,
                              getCapParam->capValueSize,
                              getCapParam->capValue);

        if( (cmRet == CM_SUCCESS) &&   getCapParam->capName == CAP_SURFACE2D_FORMATS)
        { // need to convert to OSAL Format
            cmRet = ConvertToOperatingSystemAbstractionLayerFormat(
                getCapParam->capValue, CM_MAX_SURFACE2D_FORMAT_COUNT);
        }

        if( (cmRet == CM_SUCCESS) &&  getCapParam->capName == CAP_SURFACE3D_FORMATS)
        {
            cmRet = ConvertToOperatingSystemAbstractionLayerFormat(
                getCapParam->capValue, CM_MAX_SURFACE3D_FORMAT_COUNT);
        }
        getCapParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_SETCAPS:
        PCM_DEVICE_SETCAP_PARAM setCapParam;
        setCapParam = (PCM_DEVICE_SETCAP_PARAM)(cmPrivateInputData);
        cmRet = device->SetCaps(setCapParam->capName, setCapParam->capValueSize, setCapParam->capValue);
        setCapParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_SETSUGGESTEDL3CONFIG:
        PCM_DEVICE_SETSUGGESTEDL3_PARAM setL3IndexParam;
        setL3IndexParam = (PCM_DEVICE_SETSUGGESTEDL3_PARAM)(cmPrivateInputData);
        cmRet = device->SetSuggestedL3Config(setL3IndexParam->l3SuggestConfig);
        setL3IndexParam->returnValue = cmRet;
        break;

    case CM_FN_CMQUEUE_ENQUEUECOPY:

        CM_ENQUEUE_GPUCOPY_PARAM *enqueueGpuCopyParam;
        enqueueGpuCopyParam = (CM_ENQUEUE_GPUCOPY_PARAM*)(cmPrivateInputData);
        cmQueue     =  (CmQueue *)enqueueGpuCopyParam->queueHandle;
        cmQueueRT   = static_cast<CmQueueRT*>(cmQueue);
        cmSurface2d =  CM_SURFACE_2D(enqueueGpuCopyParam->surface2d);
        cmEvent     =  (CmEvent*)enqueueGpuCopyParam->eventHandle; // used as input

        cmRet = cmQueueRT->EnqueueCopyInternal( cmSurface2d,
                                               (unsigned char *) enqueueGpuCopyParam->sysMem,
                                               enqueueGpuCopyParam->widthStride,
                                               enqueueGpuCopyParam->heightStride,
                                               enqueueGpuCopyParam->copyDir,
                                               enqueueGpuCopyParam->option,
                                               cmEvent);

        enqueueGpuCopyParam->eventHandle = cmEvent;
        enqueueGpuCopyParam->returnValue = cmRet;

        break;

   case CM_FN_CMQUEUE_ENQUEUESURF2DINIT:
        PCM_ENQUEUE_2DINIT_PARAM  enqueue2DInitParam;
        enqueue2DInitParam = (PCM_ENQUEUE_2DINIT_PARAM)(cmPrivateInputData);

        cmQueue = (CmQueue *)enqueue2DInitParam->queueHandle;
        cmSurf2DBase = (CmSurface2D *)(enqueue2DInitParam->surface2d);
        cmEvent     = (CmEvent*)enqueue2DInitParam->eventHandle; // used as input
        CM_ASSERT(cmQueue);
        CM_ASSERT(cmSurf2DBase);

        cmRet = cmQueue->EnqueueInitSurface2D(cmSurf2DBase, enqueue2DInitParam->initValue, cmEvent);

        enqueue2DInitParam->eventHandle = cmEvent;
        enqueue2DInitParam->returnValue = cmRet;
        break;

    case CM_FN_CMQUEUE_ENQUEUECOPY_V2V:
        PCM_ENQUEUE_GPUCOPY_V2V_PARAM  enqueueCopyV2VParam;
        enqueueCopyV2VParam = (PCM_ENQUEUE_GPUCOPY_V2V_PARAM)(cmPrivateInputData);

        cmQueue = (CmQueue *)enqueueCopyV2VParam->queueHandle;
        cmSrcSurface2d = (CmSurface2D *)(enqueueCopyV2VParam->srcSurface2d);
        cmDstSurface2d = (CmSurface2D *)(enqueueCopyV2VParam->dstSurface2d);
        cmEvent        = (CmEvent*)enqueueCopyV2VParam->eventHandle; // used as input
        CM_ASSERT(cmQueue);
        CM_ASSERT(cmSrcSurface2d);
        CM_ASSERT(cmDstSurface2d);

        cmRet = cmQueue->EnqueueCopyGPUToGPU(cmDstSurface2d,
                                              cmSrcSurface2d,
                                              enqueueCopyV2VParam->option,
                                              cmEvent);

        enqueueCopyV2VParam->eventHandle = cmEvent;
        enqueueCopyV2VParam->returnValue = cmRet;

        break;

    case CM_FN_CMQUEUE_ENQUEUECOPY_L2L:
        PCM_ENQUEUE_GPUCOPY_L2L_PARAM  enqueueCopyL2LParam;
        enqueueCopyL2LParam = (PCM_ENQUEUE_GPUCOPY_L2L_PARAM)(cmPrivateInputData);

        cmQueue = (CmQueue *)enqueueCopyL2LParam->queueHandle;
        cmEvent = (CmEvent*)enqueueCopyL2LParam->eventHandle; // used as input
        CM_ASSERT(cmQueue);

        cmRet = cmQueue->EnqueueCopyCPUToCPU((unsigned char *) enqueueCopyL2LParam->dstSysMem,
                                            (unsigned char *) enqueueCopyL2LParam->srcSysMem,
                                            enqueueCopyL2LParam->copySize,
                                            enqueueCopyL2LParam->option,
                                            cmEvent);

        enqueueCopyL2LParam->eventHandle = cmEvent;
        enqueueCopyL2LParam->returnValue = cmRet;

        break;

    case CM_FN_CMQUEUE_ENQUEUECOPY_BUFFER:
        PCM_ENQUEUE_COPY_BUFFER_PARAM  enqueueCopyBtoCPUParam;
        enqueueCopyBtoCPUParam = (PCM_ENQUEUE_COPY_BUFFER_PARAM)(cmPrivateInputData);
        cmQueue = (CmQueue*)enqueueCopyBtoCPUParam->cmQueueHandle;
        CM_ASSERT(cmQueue);
        cmEvent = (CmEvent*)enqueueCopyBtoCPUParam->cmEventHandle; // used as input
        cmQueueRT = static_cast<CmQueueRT*>(cmQueue);

        cmRet = cmQueueRT->EnqueueBufferCopy((CmBuffer*)enqueueCopyBtoCPUParam->buffer,
                                            enqueueCopyBtoCPUParam->offset,
                                            (unsigned char*)enqueueCopyBtoCPUParam->sysMem,
                                            enqueueCopyBtoCPUParam->copySize,
                                            (CM_GPUCOPY_DIRECTION)enqueueCopyBtoCPUParam->copyDir,
                                            (CmEvent*)enqueueCopyBtoCPUParam->wait_event,
                                            cmEvent,
                                            enqueueCopyBtoCPUParam->option);

        enqueueCopyBtoCPUParam->cmEventHandle = cmEvent;
        enqueueCopyBtoCPUParam->returnValue = cmRet;
        break;

    case CM_FN_CMQUEUE_ENQUEUEVEBOX:
        PCM_ENQUEUE_VEBOX_PARAM enqueueVeboxParam;
        enqueueVeboxParam = (PCM_ENQUEUE_VEBOX_PARAM)(cmPrivateInputData);
        cmQueue = (CmQueue *)enqueueVeboxParam->queueHandle;
        cmVebox = (CmVebox *)enqueueVeboxParam->veboxHandle;
        cmEvent = (CmEvent *)enqueueVeboxParam->eventHandle;

        if (cmQueue)
        {
            cmRet = cmQueue->EnqueueVebox(cmVebox, cmEvent);
        }

        enqueueVeboxParam->eventHandle = cmEvent;
        enqueueVeboxParam->returnValue = cmRet;

        break;

        //Surface 3D Create/Destroy Read/Write
    case CM_FN_CMDEVICE_CREATESURFACE3D:
        PCM_CREATE_SURFACE3D_PARAM createSurf3dParam;
        createSurf3dParam = (PCM_CREATE_SURFACE3D_PARAM)(cmPrivateInputData);

        cmRet = device->CreateSurface3D(createSurf3dParam->width,
                                    createSurf3dParam->height,
                                    createSurf3dParam->depth,
                                    CmOSFmtToMosFmt(createSurf3dParam->format),
                                    cmSurface3d);
        if(cmRet == CM_SUCCESS)
        {
             createSurf3dParam->surface3DHandle  = static_cast<CmSurface3D *>(cmSurface3d);
        }
        createSurf3dParam->returnValue        = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSURFACE3D:
        PCM_DESTROY_SURFACE3D_PARAM destroySurf3dParam;
        destroySurf3dParam = (PCM_DESTROY_SURFACE3D_PARAM)(cmPrivateInputData);
        cmSurface3d     = static_cast<CmSurface3D *>(destroySurf3dParam->surface3DHandle);

        cmRet = device->DestroySurface(cmSurface3d);

        destroySurf3dParam->returnValue = cmRet;
        break;

   case CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D:
        PCM_CREATESAMPLER2D_PARAM createSampler2DParam;
        createSampler2DParam = (PCM_CREATESAMPLER2D_PARAM)(cmPrivateInputData);

        cmSurf2DBase     = (CmSurface2D *)createSampler2DParam->surface2DHandle;

        cmRet = device->CreateSamplerSurface2D(cmSurf2DBase, surfaceIndex);

        createSampler2DParam->samplerSurfIndexHandle = surfaceIndex;
        createSampler2DParam->returnValue = cmRet;
        break;

   case CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D_EX:
       PCM_CREATESAMPLER2DEX_PARAM createSampler2DExParam;
       createSampler2DExParam = (PCM_CREATESAMPLER2DEX_PARAM)(cmPrivateInputData);

       cmSurf2DBase = (CmSurface2D *)createSampler2DExParam->surface2DHandle;

       cmRet = device->CreateSamplerSurface2DEx(cmSurf2DBase, surfaceIndex, createSampler2DExParam->flag);

       createSampler2DExParam->samplerSurfIndexHandle = surfaceIndex;
       createSampler2DExParam->returnValue = cmRet;
       break;

   case CM_FN_CMDEVICE_CREATESAMPLERSURFACE2DUP:
        PCM_CREATESAMPLER2DUP_PARAM createSampler2DUPParam;
        createSampler2DUPParam = (PCM_CREATESAMPLER2DUP_PARAM)(cmPrivateInputData);

        cmSurface2dup     = (CmSurface2DUP *)createSampler2DUPParam->surface2DUPHandle;

        cmRet = device->CreateSamplerSurface2DUP(cmSurface2dup, surfaceIndex);

        createSampler2DUPParam->samplerSurfIndexHandle = surfaceIndex;
        createSampler2DUPParam->returnValue = cmRet;
        break;

   case CM_FN_CMDEVICE_CREATESAMPLERSURFACE3D:
        PCM_CREATESAMPLER3D_PARAM createSampler3DParam;
        createSampler3DParam = (PCM_CREATESAMPLER3D_PARAM)(cmPrivateInputData);

        cmSurface3d     = (CmSurface3D *)createSampler3DParam->surface3DHandle;

        cmRet = device->CreateSamplerSurface3D(cmSurface3d, surfaceIndex);

        createSampler3DParam->samplerSurfIndexHandle = surfaceIndex;
        createSampler3DParam->returnValue = cmRet;
        break;

   case CM_FN_CMDEVICE_DESTROYSAMPLERSURFACE:
        PCM_DESTROYSAMPLERSURF_PARAM destroySamplerSurfParam;
        destroySamplerSurfParam = (PCM_DESTROYSAMPLERSURF_PARAM)(cmPrivateInputData);

        surfaceIndex    = (SurfaceIndex *)destroySamplerSurfParam->samplerSurfIndexHandle;
        cmRet = device->DestroySamplerSurface(surfaceIndex );

        destroySamplerSurfParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESAMPLER8X8:
        PCM_CREATESAMPLER8x8_PARAM createSampler8x8Param;
        createSampler8x8Param = (PCM_CREATESAMPLER8x8_PARAM)(cmPrivateInputData);

        cmRet = device->CreateSampler8x8(createSampler8x8Param->sample8x8Desc ,
                                       sampler8x8);
        if(cmRet == CM_SUCCESS)
        {
             cmRet = sampler8x8->GetIndex(samplerIndex);
             createSampler8x8Param->samplerIndexHandle = samplerIndex;
             createSampler8x8Param->sampler8x8Handle = static_cast<CmSampler8x8 *>(sampler8x8);
        }
        createSampler8x8Param->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSAMPLER8X8:
        PCM_DESTROYSAMPLER8x8_PARAM destroySampler8x8Param;
        destroySampler8x8Param = (PCM_DESTROYSAMPLER8x8_PARAM)(cmPrivateInputData);

        sampler8x8         = (CmSampler8x8 *)destroySampler8x8Param->sampler8x8Handle;
        cmRet = device->DestroySampler8x8( sampler8x8);

        destroySampler8x8Param->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE:
        PCM_CREATESAMPLER8x8SURF_PARAM createSampler8x8SurfParam;
        createSampler8x8SurfParam = (PCM_CREATESAMPLER8x8SURF_PARAM)(cmPrivateInputData);

        cmRet = device->CreateSampler8x8Surface(
            (CmSurface2D *) createSampler8x8SurfParam->surf2DHandle,
            surfaceIndex,
            createSampler8x8SurfParam->sampler8x8Type,
            createSampler8x8SurfParam->sampler8x8Mode);

        createSampler8x8SurfParam->surfIndexHandle = surfaceIndex;
        createSampler8x8SurfParam->returnValue     = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE_EX:
        PCM_CREATESAMPLER8x8SURFEX_PARAM createSampler8x8SurfParamEx;
        createSampler8x8SurfParamEx = (PCM_CREATESAMPLER8x8SURFEX_PARAM)(cmPrivateInputData);

        cmRet = device->CreateSampler8x8SurfaceEx(
            (CmSurface2D *)createSampler8x8SurfParamEx->surf2DHandle,
            surfaceIndex,
            createSampler8x8SurfParamEx->sampler8x8Type,
            createSampler8x8SurfParamEx->sampler8x8Mode,
            createSampler8x8SurfParamEx->flag);

        createSampler8x8SurfParamEx->surfIndexHandle = surfaceIndex;
        createSampler8x8SurfParamEx->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSAMPLER8X8SURFACE:
        PCM_DESTROYSAMPLER8x8SURF_PARAM destroySampler8x8SurfParam;
        destroySampler8x8SurfParam = (PCM_DESTROYSAMPLER8x8SURF_PARAM)(cmPrivateInputData);

        cmRet = device->DestroySampler8x8Surface(destroySampler8x8SurfParam->surfIndexHandle);

        destroySampler8x8SurfParam->returnValue       = cmRet;
        break;

#if USE_EXTENSION_CODE
    case CM_FN_CMDEVICE_ENABLE_GTPIN:
        hr = CmThinExecuteEnableGTPin(device, cmPrivateInputData);
        break;

    case   CM_FN_CMDEVICE_REGISTER_GTPIN_MARKERS:
        hr = CmThinExecuteRegGTPinMarkers(device, cmPrivateInputData);
        break;
#endif

    case CM_FN_CMDEVICE_INIT_PRINT_BUFFER:
        PCM_DEVICE_INIT_PRINT_BUFFER_PARAM initPrintBufferParam;
        initPrintBufferParam  = (PCM_DEVICE_INIT_PRINT_BUFFER_PARAM)(cmPrivateInputData);

        cmRet = device->InitPrintBuffer(initPrintBufferParam->printBufferSize);
        if( cmRet == CM_SUCCESS)
        {
            //Return the print buffer memory to thin layer
            deviceRT->GetPrintBufferMem((unsigned char *&)initPrintBufferParam->printBufferMem);
        }

        initPrintBufferParam->returnValue     = cmRet;
        break;

    case CM_FN_CMDEVICE_FLUSH_PRINT_BUFFER:
        PCM_DEVICE_FLUSH_PRINT_BUFFER_PARAM flushPrintBufferParam;
        flushPrintBufferParam  = (PCM_DEVICE_FLUSH_PRINT_BUFFER_PARAM)(cmPrivateInputData);
        cmRet = device->FlushPrintBufferIntoFile(flushPrintBufferParam->fileName);
        flushPrintBufferParam->returnValue     = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATEVEBOX:
        {
            PCM_CREATEVEBOX_PARAM createVeboxParam;
            createVeboxParam = (PCM_CREATEVEBOX_PARAM) (cmPrivateInputData);
            cmRet = device->CreateVebox(cmVebox);
            if( cmRet == CM_SUCCESS && cmVebox)
            {
                createVeboxParam->veboxHandle = cmVebox;
                CmVeboxRT *veboxRT = static_cast<CmVeboxRT *>(cmVebox);
                createVeboxParam->indexInVeboxArray = veboxRT->GetIndexInVeboxArray();
            }
            createVeboxParam->returnValue = cmRet;
        }
        break;

    case CM_FN_CMDEVICE_DESTROYVEBOX:
        PCM_DESTROYVEBOX_PARAM destroyVeboxParam;
        destroyVeboxParam = (PCM_DESTROYVEBOX_PARAM)(cmPrivateInputData);
        cmVebox         = (CmVebox *)destroyVeboxParam->veboxHandle;
        cmRet = device->DestroyVebox(cmVebox);
        destroyVeboxParam->returnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_GETVISAVERSION:
        CM_GET_VISA_VERSION_PARAM *getVisaVersionParam;
        getVisaVersionParam = (CM_GET_VISA_VERSION_PARAM *)(cmPrivateInputData);
        cmRet = device->GetVISAVersion(getVisaVersionParam->majorVersion, getVisaVersionParam->minorVersion);
        getVisaVersionParam->returnValue = cmRet;
        break;

    default:
        return CM_INVALID_PRIVATE_DATA;

    }
finish:
    return hr;
}
