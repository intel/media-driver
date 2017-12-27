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

#if USE_EXTENSION_CODE
#include "cm_ult.h"
#include "cm_gtpin.h"
#endif

struct CM_ENQUEUE_GPUCOPY_PARAM
{
    void *pCmQueueHandle;  // [in] CmQueue pointer in CMRT@UMD
    void *pCmSurface2d;    // [in] CmSurface2d pointer in CMRT@UMD
    void *pSysMem;         // [in] pointer of system memory
    CM_GPUCOPY_DIRECTION iCopyDir;  // [in] direction for GPUCopy: CM_FASTCOPY_GPU2CPU (0) or CM_FASTCOPY_CPU2GPU(1)
    uint32_t iWidthStride;   // [in] width stride in byte for system memory, ZERO means no setting
    uint32_t iHeightStride;  // [in] height stride in row for system memory, ZERO means no setting
    uint32_t iOption;        // [in] option passed by user, only support CM_FASTCOPY_OPTION_NONBLOCKING(0) and CM_FASTCOPY_OPTION_BLOCKING(1)
    void *pCmEventHandle;    // [in/out] return CmDevice pointer in CMRT@UMD, nullptr if the input is CM_NO_EVENT
    uint32_t iEventIndex;    // [out] index of Event in m_EventArray
    int32_t iReturnValue;    // [out] return value from CMRT@UMD
};

struct CM_CREATEBUFFER_PARAM
{
    uint32_t iSize;             // [in]  buffer size in byte
    CM_BUFFER_TYPE bufferType;  // [in]  Buffer type (Buffer, BufferUP, or Buffer SVM)
    void *pSysMem;              // [in]  Address of system memory
    void *pCmBufferHandle;      // [out] pointer to CmBuffer object in CMRT@UMD
    int32_t iReturnValue;       // [out] the return value from CMRT@UMD
    uint32_t uiReserved;        // Reserved field to ensure sizeof(CM_CREATEBUFFER_PARAM_V2) is different from sizeof(CM_CREATEBUFFER_PARAM_V1) in x64 mode
};

//*-----------------------------------------------------------------------------
//| Purpose:    Get Time in ms and used in performance measurement
//| Returns:    Current system time.
//*-----------------------------------------------------------------------------
double CmGetTimeInms()
{
    LARGE_INTEGER   Freq;
    LARGE_INTEGER   Count;
    double   Curtime;

    MOS_QueryPerformanceFrequency((uint64_t*)&Freq.QuadPart);
    MOS_QueryPerformanceCounter((uint64_t*)&Count.QuadPart);
    Curtime = (double)(1000.0 * Count.QuadPart/(double)Freq.QuadPart);

    return Curtime;
}


//*-----------------------------------------------------------------------------
//| Purpose:   Convert GMM_RESROUCE_Format got by GetCaps to OSAL format
//| Return:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t ConvertToOperatingSystemAbstractionLayerFormat(void *pSrc,
                                                       uint32_t NumOfFormats)
{
    uint32_t i = 0 ;
    if(pSrc == nullptr || NumOfFormats == 0)
    {
        CM_ASSERTMESSAGE("Error: Invalid input arguments.");
        return CM_INVALID_ARG_VALUE;
    }

    for ( i=0 ; i< NumOfFormats ; i++)
    {
        *((CM_OSAL_SURFACE_FORMAT *)pSrc+i) = CmMosFmtToOSFmt(*((CM_SURFACE_FORMAT *)pSrc+i));
    }
    return CM_SUCCESS;
}

#define CM_SET_NOFAILURE_STATUS(hr) \
        if (hr != CM_FAILURE) \
        { \
            hr = CM_SUCCESS; \
        }

#if USE_EXTENSION_CODE
extern int CmThinExecuteEnableGTPin(CmDevice *pDevice, void *pCmPrivateInputData);
extern int CmThinExecuteRegGTPinMarkers(CmDevice *pDevice, void *pCmPrivateInputData);
#endif

//*-----------------------------------------------------------------------------
//| Purpose:    CMRT thin layer library supported function execution
//| Return:     CM_SUCCESS if successful
//*-----------------------------------------------------------------------------
int32_t CmThinExecuteEx(CmDevice *pDevice,
                        CM_FUNCTION_ID CmFunctionID,
                        void *pCmPrivateInputData,
                        uint32_t CmPrivateInputDataSize)
{
    int32_t                     hr                  = CM_SUCCESS;
    int32_t                     cmRet               = CM_INVALID_PRIVATE_DATA;
    CmBuffer                    *pCmBuffer          = nullptr;
    CmBufferUP                  *pCmBufferUP        = nullptr;
    CmBufferSVM                 *pCmBufferSVM       = nullptr;
    CmSurface2DRT               *pCmSurface2d       = nullptr;
    CmSurface2DUP               *pCmSurface2dup     = nullptr;
    CmProgram                   *pCmProgram         = nullptr;
    CmKernel                    *pCmKernel          = nullptr;
    CmTask                      *pCmTask            = nullptr;
    CmQueue                     *pCmQueue           = nullptr;
    CmQueueRT                   *pCmQueueRT         = nullptr;
    CmEvent                     *pCmEvent           = nullptr;
    CmThreadSpace               *pCmTs              = nullptr;
    CmThreadGroupSpace          *pCmTGS             = nullptr;
    SurfaceIndex                *pVmeSurIndex       = nullptr;
    SurfaceIndex                *pSurfaceIndex      = nullptr;
    CmSampler                   *pSampler           = nullptr;
    SamplerIndex                *pSamplerIndex      = nullptr;
    CmThreadGroupSpace          *pThreadGrpSpace    = nullptr;
    CmSurface3D                 *pCmSurface3d       = nullptr;
    CmSampler8x8                *pSampler8x8        = nullptr;
    CmSurface2D                 *pCmSrcSurface2d    = nullptr;
    CmSurface2D                 *pCmDstSurface2d    = nullptr;
    CmSurface2D                 *pCmSurf2DBase      = nullptr;
    CmVebox                     *pCmVebox           = nullptr;
    CmDeviceRT                  *pDeviceRT          = nullptr;
    if (pCmPrivateInputData == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Null pointer.");
        return CM_INVALID_PRIVATE_DATA;
    }

    if ((CmFunctionID != CM_FN_RT_ULT_INFO)&&(pDevice == nullptr))
    {
        CM_ASSERTMESSAGE("Error: Null pointer.");
        return CM_NULL_POINTER;
    }
    pDeviceRT = static_cast<CmDeviceRT*>(pDevice);

    switch(CmFunctionID)
    {
    case CM_FN_CMDEVICE_CREATEBUFFER:
        if(CmPrivateInputDataSize == sizeof(CM_CREATEBUFFER_PARAM))
        {
            CM_CREATEBUFFER_PARAM *pCmBufferParam
                = (CM_CREATEBUFFER_PARAM*)(pCmPrivateInputData);

            //pDevice = CmGetDeviceHandle(hUMDevice);
            //CM_DDI_ASSERT(pDevice);
            if(pCmBufferParam->bufferType == CM_BUFFER_N)
            {
                //Create Buffer
                cmRet = pDevice->CreateBuffer(pCmBufferParam->iSize, pCmBuffer);
                //Create Surface Index
                if( cmRet == CM_SUCCESS)
                {
                    pCmBufferParam->pCmBufferHandle = static_cast<CmBuffer *>(pCmBuffer);
                }
                //Fill the output message
                pCmBufferParam->iReturnValue        = cmRet;
            }
            else if(pCmBufferParam->bufferType == CM_BUFFER_UP)
            { // Create Buffer Up
                cmRet = pDevice->CreateBufferUP(pCmBufferParam->iSize, 
                                             pCmBufferParam->pSysMem,
                                             pCmBufferUP);
                //Create Surface Index
                if( cmRet == CM_SUCCESS)
                {
                    pCmBufferParam->pCmBufferHandle     = pCmBufferUP;
                }
                //Fill the output message
                pCmBufferParam->iReturnValue        = cmRet;
            }
            else if (pCmBufferParam->bufferType == CM_BUFFER_SVM)
            {
                cmRet = pDevice->CreateBufferSVM(pCmBufferParam->iSize,
                                                 pCmBufferParam->pSysMem,
                                                 0,
                                                 pCmBufferSVM);
                if (cmRet == CM_SUCCESS)
                {
                    pCmBufferParam->pCmBufferHandle = pCmBufferSVM;
                }
                //Fill the output message
                pCmBufferParam->iReturnValue        = cmRet;
            }
            else //should never jump here
            {
                pCmBufferParam->iReturnValue = CM_INVALID_ARG_VALUE;
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
        PCM_DESTROYBUFFER_PARAM pCmDestBufferParam;
        pCmDestBufferParam = (PCM_DESTROYBUFFER_PARAM)(pCmPrivateInputData);

        pCmBuffer = (CmBuffer *)(pCmDestBufferParam->pCmBufferHandle);
        CM_ASSERT(pCmBuffer);

        cmRet = pDevice->DestroySurface(pCmBuffer);

        //Fill the output message
        pCmDestBufferParam->iReturnValue  = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYBUFFERUP:
        PCM_DESTROYBUFFER_PARAM pCmDestBufferUPParam;
        pCmDestBufferUPParam = (PCM_DESTROYBUFFER_PARAM)(pCmPrivateInputData);

        pCmBufferUP= (CmBufferUP *)(pCmDestBufferUPParam->pCmBufferHandle);
        CM_ASSERT(pCmBufferUP);

        cmRet = pDevice->DestroyBufferUP(pCmBufferUP);

        //Fill the output message
        pCmDestBufferUPParam->iReturnValue  = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYBUFFERSVM:
        PCM_DESTROYBUFFER_PARAM pCmDestBufferSVMParam;
        pCmDestBufferSVMParam = (PCM_DESTROYBUFFER_PARAM)(pCmPrivateInputData);

        pCmBufferSVM = (CmBufferSVM *)(pCmDestBufferSVMParam->pCmBufferHandle);
        CM_ASSERT(pCmBufferSVM);

        cmRet = pDevice->DestroyBufferSVM(pCmBufferSVM);
        //Fill the output message
        pCmDestBufferSVMParam->iReturnValue  = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESURFACE2DUP:
        PCM_CREATESURFACE2DUP_PARAM pCmCreate2DUpParam;
        pCmCreate2DUpParam = (PCM_CREATESURFACE2DUP_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->CreateSurface2DUP(pCmCreate2DUpParam->iWidth,
            pCmCreate2DUpParam->iHeight,
            CmOSFmtToMosFmt(pCmCreate2DUpParam->Format),
            pCmCreate2DUpParam->pSysMem,
            pCmSurface2dup);
        if( cmRet == CM_SUCCESS)
        {
            pCmCreate2DUpParam->pCmSurface2DUPHandle = static_cast<CmSurface2DUP *>(pCmSurface2dup);
        }

        pCmCreate2DUpParam->iReturnValue         = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSURFACE2DUP:
        PCM_DESTROYSURFACE2DUP_PARAM pCmDestroy2DUpParam;
        pCmDestroy2DUpParam = (PCM_DESTROYSURFACE2DUP_PARAM)(pCmPrivateInputData);
        pCmSurface2dup = static_cast<CmSurface2DUP *>(pCmDestroy2DUpParam->pCmSurface2DUPHandle);

        cmRet = pDevice->DestroySurface2DUP(pCmSurface2dup);

        pCmDestroy2DUpParam->iReturnValue        = cmRet;
        break;

    case CM_FN_CMDEVICE_GETSURFACE2DINFO:
        PCM_GETSURFACE2DINFO_PARAM pCmGet2DinfoParam;
        uint32_t pitch,physicalsize;
        pCmGet2DinfoParam = (PCM_GETSURFACE2DINFO_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->GetSurface2DInfo( pCmGet2DinfoParam->iWidth, 
                                        pCmGet2DinfoParam->iHeight,
                                        CmOSFmtToMosFmt(pCmGet2DinfoParam->format),
                                        pitch, 
                                        physicalsize);

        pCmGet2DinfoParam->iPitch           = pitch;
        pCmGet2DinfoParam->iPhysicalSize    = physicalsize;
        pCmGet2DinfoParam->iReturnValue     = cmRet;
        break;

     case CM_FN_CMDEVICE_CREATESURFACE2D_ALIAS:
         {
             PCM_DEVICE_CREATE_SURF2D_ALIAS_PARAM pCreateSurf2DAliasParam;
             pCreateSurf2DAliasParam = (PCM_DEVICE_CREATE_SURF2D_ALIAS_PARAM)(pCmPrivateInputData);
             CmSurface2D * pCmSurfBase = static_cast<CmSurface2D *>(pCreateSurf2DAliasParam->pCmSurface2DHandle);
             pSurfaceIndex = (SurfaceIndex*)pCreateSurf2DAliasParam->pSurfaceIndexHandle;

             cmRet = pDevice->CreateSurface2DAlias(pCmSurfBase, pSurfaceIndex);
             pCreateSurf2DAliasParam->pSurfaceIndexHandle = pSurfaceIndex;
             pCreateSurf2DAliasParam->iReturnValue = cmRet;
         }
         break;

     case CM_FN_CMDEVICE_CREATEBUFFER_ALIAS:
         {
             PCM_DEVICE_CREATE_BUFFER_ALIAS_PARAM pCreateBufferAliasParam;
             pCreateBufferAliasParam = (PCM_DEVICE_CREATE_BUFFER_ALIAS_PARAM)(pCmPrivateInputData);
             pCmBuffer= static_cast<CmBuffer *>(pCreateBufferAliasParam->pCmBufferHandle);
             pSurfaceIndex = (SurfaceIndex*)pCreateBufferAliasParam->pSurfaceIndexHandle;

             cmRet = pDevice->CreateBufferAlias(pCmBuffer, pSurfaceIndex);
             pCreateBufferAliasParam->pSurfaceIndexHandle = pSurfaceIndex;
             pCreateBufferAliasParam->iReturnValue = cmRet;
         }
         break;

     case CM_FN_CMDEVICE_CLONEKERNEL:
         {
             PCM_CLONE_KERNEL_PARAM  pCloneKernelParam;
             pCloneKernelParam = (PCM_CLONE_KERNEL_PARAM)(pCmPrivateInputData);
             CmKernel * pKernelDest = static_cast<CmKernel *>(pCloneKernelParam->pCmKernelHandleDest);
             CmKernel * pKernelSrc = static_cast<CmKernel *>(pCloneKernelParam->pCmKernelHandleSrc);
             cmRet = pDevice->CloneKernel(pKernelDest, pKernelSrc);
             pCloneKernelParam->pCmKernelHandleDest = pKernelDest;

             pCloneKernelParam->iReturnValue = cmRet;
         }
         break;

     case CM_FN_CMDEVICE_DESTROYSURFACE2D:
        {
            PCM_DESTROYSURFACE2D_PARAM pCmDestroy2DParam;

            pCmDestroy2DParam    = (PCM_DESTROYSURFACE2D_PARAM)(pCmPrivateInputData);
            CmSurface2D * pCmSurfBase = (CmSurface2D *)(pCmDestroy2DParam->pCmSurface2DHandle);

            CM_ASSERT(pCmSurfBase);

            cmRet = pDevice->DestroySurface(pCmSurfBase);
            //Fill output message
            pCmDestroy2DParam->pCmSurface2DHandle    = nullptr;
            pCmDestroy2DParam->iReturnValue          = cmRet;
        }
        break;

    case CM_FN_CMDEVICE_LOADPROGRAM:
        PCM_LOADPROGRAM_PARAM       pCmLoadProgParam;
        pCmLoadProgParam     = (PCM_LOADPROGRAM_PARAM)(pCmPrivateInputData);
        CM_ASSERT(pCmLoadProgParam->pCISACode);

        cmRet = pDevice->LoadProgram(pCmLoadProgParam->pCISACode, 
                                     pCmLoadProgParam->uiCISACodeSize, 
                                     pCmProgram, 
                                     pCmLoadProgParam->options);

        if(cmRet == CM_SUCCESS && pCmProgram)
        {
            pCmLoadProgParam->pCmProgramHandle  = static_cast<CmProgram *>(pCmProgram);
            CmProgramRT *pCmProgramRT = static_cast<CmProgramRT *>(pCmProgram);
            pCmLoadProgParam->indexInArray      = pCmProgramRT->GetProgramIndex();
        }
        pCmLoadProgParam->iReturnValue      = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYPROGRAM:
        PCM_DESTROYPROGRAM_PARAM   pCmDestoyProgParam;
        pCmDestoyProgParam = (PCM_DESTROYPROGRAM_PARAM)(pCmPrivateInputData);
        pCmProgram           = (CmProgram *)(pCmDestoyProgParam->pCmProgramHandle);

        CM_ASSERT(pCmProgram);

        cmRet = pDevice->DestroyProgram(pCmProgram);

        pCmDestoyProgParam->iReturnValue = cmRet;
        break;

     case CM_FN_CMDEVICE_CREATEKERNEL:
        PCM_CREATEKERNEL_PARAM  pCmCreateKernelParam;
        pCmCreateKernelParam =(PCM_CREATEKERNEL_PARAM)(pCmPrivateInputData);
        pCmProgram           = (CmProgram *)(pCmCreateKernelParam->pCmProgramHandle);

        cmRet = pDevice->CreateKernel(pCmProgram,
                                   pCmCreateKernelParam->pKernelName,
                                   pCmKernel, 
                                   pCmCreateKernelParam->pOptions);
        if(cmRet == CM_SUCCESS && pCmKernel)
        {
            pCmCreateKernelParam->pCmKernelHandle = static_cast< CmKernel * >(pCmKernel);
            CmKernelRT *pKernelRT = static_cast<CmKernelRT *>(pCmKernel);
            pCmCreateKernelParam->indexKernelArray = pKernelRT->GetKernelIndex();
        }
        pCmCreateKernelParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYKERNEL:
        PCM_DESTROYKERNEL_PARAM pCmDestroyKernelParam;
        pCmDestroyKernelParam = (PCM_DESTROYKERNEL_PARAM)(pCmPrivateInputData);
        pCmKernel            =(CmKernel *)(pCmDestroyKernelParam->pCmKernelHandle);
        CM_ASSERT(pCmKernel);

        cmRet = pDevice->DestroyKernel(pCmKernel);

        pCmDestroyKernelParam->iReturnValue = cmRet;
        break;


    case CM_FN_CMDEVICE_CREATETASK:
        PCM_CREATETASK_PARAM pCmCreateTaskParam;
        pCmCreateTaskParam = (PCM_CREATETASK_PARAM)(pCmPrivateInputData);
        cmRet = pDevice->CreateTask(pCmTask);

        if(cmRet == CM_SUCCESS && pCmTask)
        {
            pCmCreateTaskParam->pCmTaskHandle = pCmTask;
            CmTaskRT *pCmTaskRT = static_cast<CmTaskRT *>(pCmTask);
            pCmCreateTaskParam->iTaskIndex    = pCmTaskRT->GetIndexInTaskArray();
        }
        pCmCreateTaskParam->iReturnValue  = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYTASK:
        PCM_DESTROYTASK_PARAM pCmDestroyTaskParam;
        pCmDestroyTaskParam = (PCM_DESTROYTASK_PARAM)(pCmPrivateInputData);
        pCmTask             = (CmTask *)pCmDestroyTaskParam->pCmTaskHandle;

        cmRet = pDevice->DestroyTask(pCmTask);

        pCmDestroyTaskParam->iReturnValue = cmRet;
        break;


    case CM_FN_CMDEVICE_CREATEQUEUE:
    {
        CM_CREATEQUEUE_PARAM *pCmCreateQueParam;
        pCmCreateQueParam = (CM_CREATEQUEUE_PARAM *)(pCmPrivateInputData);

        CM_QUEUE_CREATE_OPTION QueueCreateOption = CM_DEFAULT_QUEUE_CREATE_OPTION;
        QueueCreateOption.QueueType = (CM_QUEUE_TYPE)pCmCreateQueParam->iCmQueueType;
        QueueCreateOption.RunAloneMode = pCmCreateQueParam->bCmRunAloneMode;
        cmRet = pDevice->CreateQueueEx(pCmQueue, QueueCreateOption);

        pCmCreateQueParam->iReturnValue = cmRet;
        pCmCreateQueParam->pCmQueueHandle = (pCmQueue);
        break;
    }

    case CM_FN_CMQUEUE_ENQUEUE:
        PCM_ENQUEUE_PARAM pCmEnqueueParam;
        pCmEnqueueParam = (PCM_ENQUEUE_PARAM)(pCmPrivateInputData);
        pCmQueue        = (CmQueue *)pCmEnqueueParam->pCmQueueHandle;
        pCmTask         = (CmTask  *)pCmEnqueueParam->pCmTaskHandle;
        pCmTs           = (CmThreadSpace *)pCmEnqueueParam->pCmThreadSpaceHandle;
        pCmEvent        = (CmEvent*)pCmEnqueueParam->pCmEventHandle; // used as input

        CM_ASSERT(pCmQueue);
        CM_ASSERT(pCmTask);

        cmRet = pCmQueue->Enqueue(pCmTask,pCmEvent,pCmTs);


        pCmEnqueueParam->pCmEventHandle = pCmEvent;
        pCmEnqueueParam->iReturnValue = cmRet;
        break;

     case CM_FN_CMQUEUE_ENQUEUEWITHHINTS:
        PCM_ENQUEUEHINTS_PARAM pCmEnqueueHintsParam;
        pCmEnqueueHintsParam = (PCM_ENQUEUEHINTS_PARAM)(pCmPrivateInputData);
        pCmQueue             = (CmQueue *)pCmEnqueueHintsParam->pCmQueueHandle;
        pCmTask              = (CmTask  *)pCmEnqueueHintsParam->pCmTaskHandle;
        pCmEvent             = (CmEvent *)pCmEnqueueHintsParam->pCmEventHandle; // used as input

        if(pCmQueue)
        {
            cmRet = pCmQueue->EnqueueWithHints(pCmTask, pCmEvent, pCmEnqueueHintsParam->uiHints);
        }

        pCmEnqueueHintsParam->pCmEventHandle = pCmEvent;
        pCmEnqueueHintsParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMQUEUE_DESTROYEVENT:
        PCM_DESTROYEVENT_PARAM pCmDestroyEventParam;
        pCmDestroyEventParam = (PCM_DESTROYEVENT_PARAM)(pCmPrivateInputData);
        pCmQueue        = (CmQueue *)pCmDestroyEventParam->pCmQueueHandle;
        pCmEvent        = (CmEvent *)pCmDestroyEventParam->pCmEventHandle;
        CM_ASSERT(pCmQueue);
        CM_ASSERT(pCmEvent);

        cmRet = pCmQueue->DestroyEvent(pCmEvent);

        pCmDestroyEventParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATETHREADSPACE:
        PCM_CREATETHREADSPACE_PARAM pCmCreateTsParam;
        pCmCreateTsParam = (PCM_CREATETHREADSPACE_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->CreateThreadSpace(pCmCreateTsParam->TsWidth, 
                                        pCmCreateTsParam->TsHeight, 
                                        pCmTs);
        if(cmRet==CM_SUCCESS && pCmTs)
        {
            pCmCreateTsParam->pCmTsHandle = pCmTs;
            CmThreadSpaceRT *pCmTsRT = static_cast<CmThreadSpaceRT *>(pCmTs);
            pCmCreateTsParam->indexInTSArray = pCmTsRT->GetIndexInTsArray();
        }
        pCmCreateTsParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYTHREADSPACE:
        PCM_DESTROYTHREADSPACE_PARAM pCmDestroyTsParam;
        pCmDestroyTsParam = (PCM_DESTROYTHREADSPACE_PARAM)(pCmPrivateInputData);
        pCmTs               = (CmThreadSpace *)pCmDestroyTsParam->pCmTsHandle;
        CM_ASSERT(pCmTs);
        cmRet = pDevice->DestroyThreadSpace(pCmTs);
        pCmDestroyTsParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATEVMESURFACEG7_5:
        PCM_CREATEVMESURFACE_PARAM  pCreateVmeSurf7p5Param;
        pCreateVmeSurf7p5Param = ( PCM_CREATEVMESURFACE_PARAM )( pCmPrivateInputData );

        cmRet = pDevice->CreateVmeSurfaceG7_5((CmSurface2D *) pCreateVmeSurf7p5Param->pCmCurSurfHandle,
                                           (CmSurface2D * *) pCreateVmeSurf7p5Param->pCmForwardSurfArray, 
                                           (CmSurface2D * * )pCreateVmeSurf7p5Param->pCmBackwardSurfArray, 
                                           pCreateVmeSurf7p5Param->iForwardSurfCount,
                                           pCreateVmeSurf7p5Param->iBackwardSurfCount,
                                           pVmeSurIndex);

        pCreateVmeSurf7p5Param->pCmVmeSurfIndexHandle = pVmeSurIndex;
        pCreateVmeSurf7p5Param->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATEHEVCVMESURFACEG10:
        PCM_CREATEVMESURFACE_PARAM  pCreateVmeSurfParamG10;
        pCreateVmeSurfParamG10 = ( PCM_CREATEVMESURFACE_PARAM )( pCmPrivateInputData );

        cmRet = pDevice->CreateHevcVmeSurfaceG10( ( CmSurface2D * )pCreateVmeSurfParamG10->pCmCurSurfHandle,
                                             ( CmSurface2D * * )pCreateVmeSurfParamG10->pCmForwardSurfArray,
                                             ( CmSurface2D * * )pCreateVmeSurfParamG10->pCmBackwardSurfArray,
                                             pCreateVmeSurfParamG10->iForwardSurfCount,
                                             pCreateVmeSurfParamG10->iBackwardSurfCount,
                                             pVmeSurIndex );

        pCreateVmeSurfParamG10->pCmVmeSurfIndexHandle = pVmeSurIndex;
        pCreateVmeSurfParamG10->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYVMESURFACE:
        PCM_DESTROYVMESURFACE_PARAM pDestroyVmeSurfParam;
        pDestroyVmeSurfParam = ( PCM_DESTROYVMESURFACE_PARAM )( pCmPrivateInputData );

        pVmeSurIndex = ( SurfaceIndex* )pDestroyVmeSurfParam->pCmVmeSurfIndexHandle;
        cmRet = pDeviceRT->DestroyVmeSurface( pVmeSurIndex );
        pDestroyVmeSurfParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CONFIGVMESURFACEDIMENSION:
        PCM_CONFIGVMESURFACEDIMENSION_PARAM pConfigSurfStateParam;
        pConfigSurfStateParam = (PCM_CONFIGVMESURFACEDIMENSION_PARAM)(pCmPrivateInputData);

        pVmeSurIndex = (SurfaceIndex* )pConfigSurfStateParam->pCmVmeSurfHandle;
        cmRet = pDevice->SetVmeSurfaceStateParam(pVmeSurIndex, pConfigSurfStateParam->pSurfDimPara);
        pConfigSurfStateParam->iReturnValue = cmRet;
        break;
        
    case CM_FN_CMDEVICE_CREATESAMPLER:
        PCM_CREATESAMPLER_PARAM pCreateSamplerParam;
        pCreateSamplerParam = (PCM_CREATESAMPLER_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->CreateSampler(pCreateSamplerParam->SampleState, pSampler);
        if(cmRet == CM_SUCCESS)
        {
             cmRet = pSampler->GetIndex(pSamplerIndex);
             pCreateSamplerParam->pCmSamplerHandle =  static_cast<CmSampler *>(pSampler);
             pCreateSamplerParam->pCmSamplerIndexHandle = pSamplerIndex;
        }
        pCreateSamplerParam->iReturnValue = cmRet;
        break;
    
    case CM_FN_CMDEVICE_CREATESAMPLER_EX:
        PCM_CREATESAMPLER_PARAM_EX pCreateSamplerParamEx;
        pCreateSamplerParamEx = (PCM_CREATESAMPLER_PARAM_EX)(pCmPrivateInputData);

        cmRet = pDevice->CreateSamplerEx(pCreateSamplerParamEx->SampleState, pSampler);
        if(cmRet == CM_SUCCESS)
        {
             cmRet = pSampler->GetIndex(pSamplerIndex);
             pCreateSamplerParamEx->pCmSamplerHandle =  static_cast<CmSampler *>(pSampler);
             pCreateSamplerParamEx->pCmSamplerIndexHandle = pSamplerIndex;
        }
        pCreateSamplerParamEx->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSAMPLER:
        PCM_DESTROYSAMPLER_PARAM pDestroySamplerParam;
        pDestroySamplerParam = (PCM_DESTROYSAMPLER_PARAM)(pCmPrivateInputData);

        pSampler            = (CmSampler *)pDestroySamplerParam->pCmSamplerHandle;
        cmRet = pDevice->DestroySampler(pSampler);
        pDestroySamplerParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATETHREADGROUPSPACE:
        PCM_CREATETGROUPSPACE_PARAM pCreateTGrpSpaceParam;
        pCreateTGrpSpaceParam = (PCM_CREATETGROUPSPACE_PARAM)(pCmPrivateInputData);

        cmRet  = pDevice->CreateThreadGroupSpaceEx(
                 pCreateTGrpSpaceParam->thrdSpaceWidth,
                 pCreateTGrpSpaceParam->thrdSpaceHeight,
                 pCreateTGrpSpaceParam->thrdSpaceDepth,
                 pCreateTGrpSpaceParam->grpSpaceWidth,
                 pCreateTGrpSpaceParam->grpSpaceHeight,
                 pCreateTGrpSpaceParam->grpSpaceDepth,
                 pThreadGrpSpace);
        if(cmRet == CM_SUCCESS && pThreadGrpSpace)
        {
            pCreateTGrpSpaceParam->pCmGrpSpaceHandle = static_cast<CmThreadGroupSpace *>(pThreadGrpSpace);
            pCreateTGrpSpaceParam->iTGSIndex         = pThreadGrpSpace->GetIndexInTGsArray();
        }
        pCreateTGrpSpaceParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYTHREADGROUPSPACE:
        PCM_DESTROYTGROPUSPACE_PARAM pDestroyTGrpSpaceParam;
        pDestroyTGrpSpaceParam = (PCM_DESTROYTGROPUSPACE_PARAM)(pCmPrivateInputData);

        pThreadGrpSpace     = (CmThreadGroupSpace *)pDestroyTGrpSpaceParam->pCmGrpSpaceHandle;
        cmRet = pDevice->DestroyThreadGroupSpace(pThreadGrpSpace);
        pDestroyTGrpSpaceParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMQUEUE_ENQUEUEWITHGROUP:
        PCM_ENQUEUEGROUP_PARAM pEnqueueGroupParam;
        pEnqueueGroupParam = (PCM_ENQUEUEGROUP_PARAM)(pCmPrivateInputData);
        pCmQueue    = (CmQueue *)pEnqueueGroupParam->pCmQueueHandle;
        pThreadGrpSpace = (CmThreadGroupSpace *)pEnqueueGroupParam->pCmTGrpSpaceHandle;
        pCmTask     = (CmTask *)pEnqueueGroupParam->pCmTaskHandle;
        pCmEvent    = (CmEvent*)pEnqueueGroupParam->pCmEventHandle; // used as input

        cmRet = pCmQueue->EnqueueWithGroup(pCmTask, 
                                        pCmEvent, 
                                        pThreadGrpSpace);


        pEnqueueGroupParam->pCmEventHandle = pCmEvent;
        pEnqueueGroupParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_GETCAPS:
        PCM_GETCAPS_PARAM pGetCapParam;
        pGetCapParam = (PCM_GETCAPS_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->GetCaps(pGetCapParam->capName,
                              pGetCapParam->capValueSize, 
                              pGetCapParam->pCapValue);

        if( (cmRet == CM_SUCCESS) &&   pGetCapParam->capName == CAP_SURFACE2D_FORMATS)
        { // need to convert to OSAL Format
            cmRet = ConvertToOperatingSystemAbstractionLayerFormat(
                pGetCapParam->pCapValue, CM_MAX_SURFACE2D_FORMAT_COUNT);
        }

        if( (cmRet == CM_SUCCESS) &&  pGetCapParam->capName == CAP_SURFACE3D_FORMATS)
        {
            cmRet = ConvertToOperatingSystemAbstractionLayerFormat(
                pGetCapParam->pCapValue, CM_MAX_SURFACE3D_FORMAT_COUNT);
        }
        pGetCapParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_SETCAPS:
        PCM_DEVICE_SETCAP_PARAM pSetCapParam;
        pSetCapParam = (PCM_DEVICE_SETCAP_PARAM)(pCmPrivateInputData);
        cmRet = pDevice->SetCaps(pSetCapParam->capName, pSetCapParam->capValueSize, pSetCapParam->pCapValue);
        pSetCapParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_SETSUGGESTEDL3CONFIG:
        PCM_DEVICE_SETSUGGESTEDL3_PARAM pSetL3IndexParam;
        pSetL3IndexParam = (PCM_DEVICE_SETSUGGESTEDL3_PARAM)(pCmPrivateInputData);
        cmRet = pDevice->SetSuggestedL3Config(pSetL3IndexParam->l3_s_c);
        pSetL3IndexParam->iReturnValue = cmRet;
        break;
        
    case CM_FN_CMQUEUE_ENQUEUECOPY:

        CM_ENQUEUE_GPUCOPY_PARAM *pEnqueueGpuCopyParam;
        pEnqueueGpuCopyParam = (CM_ENQUEUE_GPUCOPY_PARAM*)(pCmPrivateInputData);
        pCmQueue     =  (CmQueue *)pEnqueueGpuCopyParam->pCmQueueHandle;
        pCmQueueRT   = static_cast<CmQueueRT*>(pCmQueue);
        pCmSurface2d =  CM_SURFACE_2D(pEnqueueGpuCopyParam->pCmSurface2d);
        pCmEvent     =  (CmEvent*)pEnqueueGpuCopyParam->pCmEventHandle; // used as input

        cmRet = pCmQueueRT->EnqueueCopyInternal( pCmSurface2d,
                                               (unsigned char *) pEnqueueGpuCopyParam->pSysMem, 
                                               pEnqueueGpuCopyParam->iWidthStride, 
                                               pEnqueueGpuCopyParam->iHeightStride,
                                               pEnqueueGpuCopyParam->iCopyDir,
                                               pEnqueueGpuCopyParam->iOption,
                                               pCmEvent);


        pEnqueueGpuCopyParam->pCmEventHandle = pCmEvent;
        pEnqueueGpuCopyParam->iReturnValue   = cmRet;


        break;

   case CM_FN_CMQUEUE_ENQUEUESURF2DINIT:
        PCM_ENQUEUE_2DINIT_PARAM  pEnqueue2DInitParam;
        pEnqueue2DInitParam = (PCM_ENQUEUE_2DINIT_PARAM)(pCmPrivateInputData);

        pCmQueue = (CmQueue *)pEnqueue2DInitParam->pCmQueueHandle;
        pCmSurf2DBase = (CmSurface2D *)(pEnqueue2DInitParam->pCmSurface2d);
        pCmEvent     = (CmEvent*)pEnqueue2DInitParam->pCmEventHandle; // used as input
        CM_ASSERT(pCmQueue);
        CM_ASSERT(pCmSurf2DBase);

        cmRet = pCmQueue->EnqueueInitSurface2D(pCmSurf2DBase, pEnqueue2DInitParam->dwInitValue, pCmEvent);

        pEnqueue2DInitParam->pCmEventHandle = pCmEvent;        
        pEnqueue2DInitParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMQUEUE_ENQUEUECOPY_V2V: 
        PCM_ENQUEUE_GPUCOPY_V2V_PARAM  pEnqueueCopyV2VParam;
        pEnqueueCopyV2VParam = (PCM_ENQUEUE_GPUCOPY_V2V_PARAM)(pCmPrivateInputData);

        pCmQueue = (CmQueue *)pEnqueueCopyV2VParam->pCmQueueHandle;
        pCmSrcSurface2d = (CmSurface2D *)(pEnqueueCopyV2VParam->pCmSrcSurface2d);
        pCmDstSurface2d = (CmSurface2D *)(pEnqueueCopyV2VParam->pCmDstSurface2d);
        pCmEvent        = (CmEvent*)pEnqueueCopyV2VParam->pCmEventHandle; // used as input
        CM_ASSERT(pCmQueue);
        CM_ASSERT(pCmSrcSurface2d);
        CM_ASSERT(pCmDstSurface2d);

        cmRet = pCmQueue->EnqueueCopyGPUToGPU(pCmDstSurface2d, 
                                              pCmSrcSurface2d,
                                              pEnqueueCopyV2VParam->iOption,
                                              pCmEvent);

        pEnqueueCopyV2VParam->pCmEventHandle = pCmEvent;
        pEnqueueCopyV2VParam->iReturnValue = cmRet;

        break;

    case CM_FN_CMQUEUE_ENQUEUECOPY_L2L: 
        PCM_ENQUEUE_GPUCOPY_L2L_PARAM  pEnqueueCopyL2LParam;
        pEnqueueCopyL2LParam = (PCM_ENQUEUE_GPUCOPY_L2L_PARAM)(pCmPrivateInputData);

        pCmQueue = (CmQueue *)pEnqueueCopyL2LParam->pCmQueueHandle;
        pCmEvent = (CmEvent*)pEnqueueCopyL2LParam->pCmEventHandle; // used as input
        CM_ASSERT(pCmQueue);

        cmRet = pCmQueue->EnqueueCopyCPUToCPU((unsigned char *) pEnqueueCopyL2LParam->pDstSysMem, 
                                           (unsigned char *) pEnqueueCopyL2LParam->pSrcSysMem, 
                                            pEnqueueCopyL2LParam->CopySize,
                                            pEnqueueCopyL2LParam->iOption,
                                            pCmEvent);

        pEnqueueCopyL2LParam->pCmEventHandle = pCmEvent;
        pEnqueueCopyL2LParam->iReturnValue = cmRet;

        break;

    case CM_FN_CMQUEUE_ENQUEUEVEBOX:
        PCM_ENQUEUE_VEBOX_PARAM pEnqueueVeboxParam;
        pEnqueueVeboxParam = (PCM_ENQUEUE_VEBOX_PARAM)(pCmPrivateInputData);
        pCmQueue = (CmQueue *)pEnqueueVeboxParam->pCmQueueHandle;
        pCmVebox = (CmVebox *)pEnqueueVeboxParam->pCmVeboxHandle;
        pCmEvent = (CmEvent *)pEnqueueVeboxParam->pCmEventHandle;

        if (pCmQueue)
        {
            cmRet = pCmQueue->EnqueueVebox(pCmVebox, pCmEvent);
        }


        pEnqueueVeboxParam->pCmEventHandle = pCmEvent;

        pEnqueueVeboxParam->iReturnValue = cmRet;

        break;

        //Surface 3D Create/Destroy Read/Write
    case CM_FN_CMDEVICE_CREATESURFACE3D:
        PCM_CREATE_SURFACE3D_PARAM pCreateS3dParam;
        pCreateS3dParam = (PCM_CREATE_SURFACE3D_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->CreateSurface3D(pCreateS3dParam->iWidth, 
                                    pCreateS3dParam->iHeight, 
                                    pCreateS3dParam->iDepth, 
                                    CmOSFmtToMosFmt(pCreateS3dParam->Format),
                                    pCmSurface3d);
        if(cmRet == CM_SUCCESS)
        {
             pCreateS3dParam->pCmSurface3DHandle  = static_cast<CmSurface3D *>(pCmSurface3d);
        }
        pCreateS3dParam->iReturnValue        = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSURFACE3D:
        PCM_DESTROY_SURFACE3D_PARAM pDestroyS3dParam;
        pDestroyS3dParam = (PCM_DESTROY_SURFACE3D_PARAM)(pCmPrivateInputData);
        pCmSurface3d     = static_cast<CmSurface3D *>(pDestroyS3dParam->pCmSurface3DHandle);

        cmRet = pDevice->DestroySurface(pCmSurface3d);

        pDestroyS3dParam->iReturnValue = cmRet;
        break;

   case CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D:
        PCM_CREATESAMPLER2D_PARAM pCreateSSurf2DParam;
        pCreateSSurf2DParam = (PCM_CREATESAMPLER2D_PARAM)(pCmPrivateInputData);

        pCmSurf2DBase     = (CmSurface2D *)pCreateSSurf2DParam->pCmSurface2DHandle;

        cmRet = pDevice->CreateSamplerSurface2D(pCmSurf2DBase, pSurfaceIndex);

        pCreateSSurf2DParam->pSamplerSurfIndexHandle = pSurfaceIndex;
        pCreateSSurf2DParam->iReturnValue = cmRet;
        break;

   case CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D_EX:
       PCM_CREATESAMPLER2DEX_PARAM pCreateSSurf2DExParam;
       pCreateSSurf2DExParam = (PCM_CREATESAMPLER2DEX_PARAM)(pCmPrivateInputData);

       pCmSurf2DBase = (CmSurface2D *)pCreateSSurf2DExParam->pCmSurface2DHandle;

       cmRet = pDevice->CreateSamplerSurface2DEx(pCmSurf2DBase, pSurfaceIndex, pCreateSSurf2DExParam->pFlag);

       pCreateSSurf2DExParam->pSamplerSurfIndexHandle = pSurfaceIndex;
       pCreateSSurf2DExParam->iReturnValue = cmRet;
       break;

   case CM_FN_CMDEVICE_CREATESAMPLERSURFACE2DUP:
        PCM_CREATESAMPLER2DUP_PARAM pCreateSurf2DUPParam;
        pCreateSurf2DUPParam = (PCM_CREATESAMPLER2DUP_PARAM)(pCmPrivateInputData);

        pCmSurface2dup     = (CmSurface2DUP *)pCreateSurf2DUPParam->pCmSurface2DUPHandle;

        cmRet = pDevice->CreateSamplerSurface2DUP(pCmSurface2dup, pSurfaceIndex);

        pCreateSurf2DUPParam->pSamplerSurfIndexHandle = pSurfaceIndex;
        pCreateSurf2DUPParam->iReturnValue = cmRet;
        break;

   case CM_FN_CMDEVICE_CREATESAMPLERSURFACE3D:
        PCM_CREATESAMPLER3D_PARAM pCreateSSurf3DParam;
        pCreateSSurf3DParam = (PCM_CREATESAMPLER3D_PARAM)(pCmPrivateInputData);

        pCmSurface3d     = (CmSurface3D *)pCreateSSurf3DParam->pCmSurface3DHandle;

        cmRet = pDevice->CreateSamplerSurface3D(pCmSurface3d, pSurfaceIndex);

        pCreateSSurf3DParam->pSamplerSurfIndexHandle = pSurfaceIndex;
        pCreateSSurf3DParam->iReturnValue = cmRet;
        break;

   case CM_FN_CMDEVICE_DESTROYSAMPLERSURFACE:
        PCM_DESTROYSAMPLERSURF_PARAM pDestroySSurfParam;
        pDestroySSurfParam = (PCM_DESTROYSAMPLERSURF_PARAM)(pCmPrivateInputData);

        pSurfaceIndex    = (SurfaceIndex *)pDestroySSurfParam->pSamplerSurfIndexHandle;
        cmRet = pDevice->DestroySamplerSurface(pSurfaceIndex );

        pDestroySSurfParam->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESAMPLER8X8:
        PCM_CREATESAMPLER8x8_PARAM pCreateSam8x8Param;
        pCreateSam8x8Param = (PCM_CREATESAMPLER8x8_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->CreateSampler8x8(pCreateSam8x8Param->Sample8x8Desc ,
                                       pSampler8x8);
        if(cmRet == CM_SUCCESS)
        {
             cmRet = pSampler8x8->GetIndex(pSamplerIndex);
             pCreateSam8x8Param->pCmSamplerIndexHandle = pSamplerIndex;
             pCreateSam8x8Param->pCmSampler8x8Handle = static_cast<CmSampler8x8 *>(pSampler8x8);
        }
        pCreateSam8x8Param->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSAMPLER8X8:
        PCM_DESTROYSAMPLER8x8_PARAM pDestroySam8x8Param;
        pDestroySam8x8Param = (PCM_DESTROYSAMPLER8x8_PARAM)(pCmPrivateInputData);

        pSampler8x8         = (CmSampler8x8 *)pDestroySam8x8Param->pCmSampler8x8Handle;
        cmRet = pDevice->DestroySampler8x8( pSampler8x8);

        pDestroySam8x8Param->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE:
        PCM_CREATESAMPLER8x8SURF_PARAM pCreateSam8x8SurfParam;
        pCreateSam8x8SurfParam = (PCM_CREATESAMPLER8x8SURF_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->CreateSampler8x8Surface(
            (CmSurface2D *) pCreateSam8x8SurfParam->pCmSurf2DHandle, 
            pSurfaceIndex,
            pCreateSam8x8SurfParam->CmSampler8x8Type,
            pCreateSam8x8SurfParam->Sampler8x8Mode);

        pCreateSam8x8SurfParam->pCmSurfIndexHandle = pSurfaceIndex;
        pCreateSam8x8SurfParam->iReturnValue       = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE_EX:
        PCM_CREATESAMPLER8x8SURFEX_PARAM pCreateSam8x8SurfParamEx;
        pCreateSam8x8SurfParamEx = (PCM_CREATESAMPLER8x8SURFEX_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->CreateSampler8x8SurfaceEx(
            (CmSurface2D *)pCreateSam8x8SurfParamEx->pCmSurf2DHandle,
            pSurfaceIndex,
            pCreateSam8x8SurfParamEx->CmSampler8x8Type,
            pCreateSam8x8SurfParamEx->Sampler8x8Mode,
            pCreateSam8x8SurfParamEx->pFlag);

        pCreateSam8x8SurfParamEx->pCmSurfIndexHandle = pSurfaceIndex;
        pCreateSam8x8SurfParamEx->iReturnValue = cmRet;
        break;

    case CM_FN_CMDEVICE_DESTROYSAMPLER8X8SURFACE:
        PCM_DESTROYSAMPLER8x8SURF_PARAM pDestroySam8x8SurfParam;
        pDestroySam8x8SurfParam = (PCM_DESTROYSAMPLER8x8SURF_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->DestroySampler8x8Surface(pDestroySam8x8SurfParam->pCmSurfIndexHandle);

        pDestroySam8x8SurfParam->iReturnValue       = cmRet;
        break;
        
#if USE_EXTENSION_CODE
    case CM_FN_CMDEVICE_ENABLE_GTPIN:
        hr = CmThinExecuteEnableGTPin(pDevice, pCmPrivateInputData);
        break;

    case   CM_FN_CMDEVICE_REGISTER_GTPIN_MARKERS:
        hr = CmThinExecuteRegGTPinMarkers(pDevice, pCmPrivateInputData);
        break;
#endif

    case CM_FN_CMDEVICE_INIT_PRINT_BUFFER:
        PCM_DEVICE_INIT_PRINT_BUFFER_PARAM pInitPrintBufferParam;
        pInitPrintBufferParam  = (PCM_DEVICE_INIT_PRINT_BUFFER_PARAM)(pCmPrivateInputData);

        cmRet = pDevice->InitPrintBuffer(pInitPrintBufferParam->dwPrintBufferSize);
        if( cmRet == CM_SUCCESS)
        {
            //Return the print buffer memory to thin layer
            pDeviceRT->GetPrintBufferMem((unsigned char *&)pInitPrintBufferParam->pPrintBufferMem);
        }

        pInitPrintBufferParam->iReturnValue     = cmRet;
        break;

    case CM_FN_CMDEVICE_CREATEVEBOX:
        {
            PCM_CREATEVEBOX_PARAM pCreateVeboxParam;
            pCreateVeboxParam = (PCM_CREATEVEBOX_PARAM) (pCmPrivateInputData);
            cmRet = pDevice->CreateVebox(pCmVebox);
            if( cmRet == CM_SUCCESS && pCmVebox)
            {
                pCreateVeboxParam->pCmVeboxHandle = pCmVebox;
                CmVeboxRT *pVeboxRT = static_cast<CmVeboxRT *>(pCmVebox);
                pCreateVeboxParam->indexInVeboxArray = pVeboxRT->GetIndexInVeboxArray();
            }
            pCreateVeboxParam->iReturnValue = cmRet;
        }
        break;
        
    case CM_FN_CMDEVICE_DESTROYVEBOX:
        PCM_DESTROYVEBOX_PARAM pDestroyVeboxParam;
        pDestroyVeboxParam = (PCM_DESTROYVEBOX_PARAM)(pCmPrivateInputData);
        pCmVebox         = (CmVebox *)pDestroyVeboxParam->pCmVeboxHandle;
        cmRet = pDevice->DestroyVebox(pCmVebox);
        pDestroyVeboxParam->iReturnValue = cmRet;
        break;

#if USE_EXTENSION_CODE
        //Get MDF UMD ULT information
    case CM_FN_RT_ULT_INFO:
        hr = CmThinExecuteUltInfo(pCmPrivateInputData);
        break;
#endif

    case CM_FN_CMDEVICE_GETVISAVERSION:
        CM_GET_VISA_VERSION_PARAM *pGetVISAVersionParam;
        pGetVISAVersionParam = (CM_GET_VISA_VERSION_PARAM *)(pCmPrivateInputData);
        cmRet = pDevice->GetVISAVersion(pGetVISAVersionParam->iMajorVersion, pGetVISAVersionParam->iMinorVersion);
        pGetVISAVersionParam->iReturnValue = cmRet;
        break;

    default:
        CM_ASSERTMESSAGE("Error: Invalid Function code '%d'.", CmFunctionID);
        hr = CM_INVALID_PRIVATE_DATA;
        break;

    }
finish:
    return hr;
}
