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
//! \file      cm_wrapper.h  
//! \brief     Contains CM Device creation/destory definitionsthe and function tables for CM Ults  
//!
#ifndef __CM_WRAPPER_H__
#define __CM_WRAPPER_H__

#include "cm_func.h"
#include "cm_device.h"
#include "cm_program.h"
#include "cm_kernel.h"
#include "cm_surface_2d_up.h"
#include "cm_surface_3d.h"
#include "cm_buffer.h"
#include "cm_event.h"
#include "cm_task.h"
#include "cm_queue.h"
#include "cm_def.h"
#include "cm_thread_space.h"
#include "cm_surface_vme.h"
#include "cm_sampler.h"
#include "cm_surface_sampler8x8.h"
#include "cm_sampler8x8.h"
#include "cm_group_space.h"
#include "cm_surface_2d.h"
#include "cm_vebox.h"

#include "cm_wrapper_os.h"


#define CM_BOUNDARY_PIXEL_MODE                  GFX3DSTATE_MEDIA_BOUNDARY_PIXEL_MODE

#define CM_SURFACE_2D(pSurf)                    static_cast<CmSurface2DRT*>((CmSurface2D *)(pSurf))

//////////////////////////////////////////////////////////////////////////////////////
// Thin CMRT definition -- START
//////////////////////////////////////////////////////////////////////////////////////
typedef struct _CM_DESTROYCMDEVICE_PARAM
{
    void        *pCmDeviceHandle;        // [in/out] pointer to CmDevice object
    int32_t     iReturnValue;           // [out] the return value from CMRT@UMD
}CM_DESTROYCMDEVICE_PARAM, *PCM_DESTROYCMDEVICE_PARAM;

typedef struct _CM_CREATEBUFFER_PARAM
{
    uint32_t        iSize;                  // [in]  buffer size in byte
    CM_BUFFER_TYPE  bufferType;             // [in]  Buffer type (Buffer, BufferUP, or Buffer SVM)
    void            *pSysMem;                // [in]  Address of system memory
    void            *pCmBufferHandle;        // [out] pointer to CmBuffer object in CMRT@UMD
    int32_t         iReturnValue;           // [out] the return value from CMRT@UMD
    uint32_t        uiReserved;             // Reserved field to ensure sizeof(CM_CREATEBUFFER_PARAM_V2) is different from sizeof(CM_CREATEBUFFER_PARAM_V1) in x64 mode
}CM_CREATEBUFFER_PARAM, *PCM_CREATEBUFFER_PARAM;


typedef struct _CM_DESTROYBUFFER_PARAM
{
    void        *pCmBufferHandle;       // [in/out] pointer to CmBuffer object
    int32_t     iReturnValue;           // [out] the return value from CMRT@UMD
}CM_DESTROYBUFFER_PARAM, *PCM_DESTROYBUFFER_PARAM;

typedef struct _CM_DESTROYSURFACE2D_PARAM
{
    void        *pCmSurface2DHandle;         // [in/out] pointer to CmSurface2D object
    int32_t     iReturnValue;               // [out] the return value from CMRT@UMD
}CM_DESTROYSURFACE2D_PARAM, *PCM_DESTROYSURFACE2D_PARAM;

typedef struct _CM_CREATESURFACE2DUP_PARAM
{
    uint32_t iWidth;                // [in] width of 2D texture in pixel
    uint32_t iHeight;               // [in] height of 2D texture in pixel
    CM_OSAL_SURFACE_FORMAT Format;  // [in] 2D texture foramt in OS layer.
    void *pSysMem;                  // [in] Pointer to system memory
    void *pCmSurface2DUPHandle;     // [out] pointer of CmSurface2D used in driver
    int32_t iReturnValue;           // [out] the return value from driver
}CM_CREATESURFACE2DUP_PARAM, *PCM_CREATESURFACE2DUP_PARAM;

typedef struct _CM_DESTROYSURFACE2DUP_PARAM
{
    void        *pCmSurface2DUPHandle;         // [in/out] pointer to CmSurface2D object
    int32_t     iReturnValue;               // [out] the return value from CMRT@UMD
}CM_DESTROYSURFACE2DUP_PARAM, *PCM_DESTROYSURFACE2DUP_PARAM;

typedef struct _CM_LOADPROGRAM_PARAM
{
    void                *pCISACode;              // [in] pointer to the CISA code buffer
    uint32_t            uiCISACodeSize;         // [in] size of CISA code
    char*               options;                // [in] additonal options for LoadProgram
    void                *pCmProgramHandle;       // [out] pointer to CmProgram object used by CMRT@UMD
    uint32_t            indexInArray;           // [out] index in m_ProgramArray of CMRT@UMD
    int32_t             iReturnValue;           // [out] the return value from CMRT@UMD
}CM_LOADPROGRAM_PARAM, *PCM_LOADPROGRAM_PARAM;

typedef struct _CM_DESTROYPROGRAM_PARAM
{
    void                *pCmProgramHandle;       // [IN] pointer to CmProgram object used by CMRT@UMD
    int32_t             iReturnValue;           // [out] the return value from CMRT@UMD
}CM_DESTROYPROGRAM_PARAM, *PCM_DESTROYPROGRAM_PARAM;

typedef struct _CM_CREATEKERNEL_PARAM
{
    void                *pCmProgramHandle;       // [in] pointer to CmProgram used in driver
    char*               pKernelName;            // [in] pointer to the kernel name string
    char*               pOptions;               // [in] pointer to the kernel creation options
    void                *pCmKernelHandle;        // [out] pointer to new created CmKernel used in driver
    uint32_t            indexKernelArray;       // [out] index in m_KernelArray
    int32_t             iReturnValue;           // [out] the return value from driver
}CM_CREATEKERNEL_PARAM, *PCM_CREATEKERNEL_PARAM;

typedef struct _CM_DESTROYKERNEL_PARAM
{
    void                *pCmKernelHandle;        // [in/out] pointer to new created CmKernel used in driver
    int32_t             iReturnValue;           // [out] the return value from driver
}CM_DESTROYKERNEL_PARAM, *PCM_DESTROYKERNEL_PARAM;

typedef struct _CM_SETSURFACEMEMORYOBJECTCTRL_PARAM
{
    void                    *pCmSurfaceHandle;       // [in]
    MEMORY_OBJECT_CONTROL   memCtrl;                // [in]
    MEMORY_TYPE             memType;                // [in]
    uint32_t                uiAge;                  // [in]
    int32_t                 iReturnValue;           // [out]
}CM_SETSURFACEMEMORYOBJECTCTRL_PARAM, *PCM_SETSURFACEMEMORYOBJECTCTRL_PARAM;

typedef struct _CM_CREATETASK_PARAM
{
    void                *pCmTaskHandle;          // [out] pointer to new created CmTask used in driver
    uint32_t            iTaskIndex;             // [out] index of task i
    int32_t             iReturnValue;           // [out] the return value from driver
}CM_CREATETASK_PARAM, *PCM_CREATETASK_PARAM;

typedef struct _CM_DESTROYTASK_PARAM
{
    void                *pCmTaskHandle;          // [in/out] pointer to CmTask used in driver
    int32_t             iReturnValue;           // [out] the return value from driver
}CM_DESTROYTASK_PARAM, *PCM_DESTROYTASK_PARAM;

struct CM_CREATEQUEUE_PARAM
{
    unsigned int            iCmQueueType;           // [in]
    bool                    bCmRunAloneMode;        // [in]
    unsigned int            iCmGPUContext;          // [in]
    void                   *pCmQueueHandle;         // [out]
    int32_t                 iReturnValue;           // [out]
};

typedef struct _CM_ENQUEUE_PARAM
{
    void                *pCmQueueHandle;         // [in]
    void                *pCmTaskHandle;          // [in]
    void                *pCmThreadSpaceHandle;   // [in]
    void                *pCmEventHandle;         // [out]
    uint32_t            iEventIndex;            // [out] index of pCmEventHandle in m_EventArray
    int32_t             iReturnValue;           // [out]               
}CM_ENQUEUE_PARAM, *PCM_ENQUEUE_PARAM;

typedef struct _CM_ENQUEUEHINTS_PARAM
{
    void                 *pCmQueueHandle;        // [in]
    void                 *pCmTaskHandle;         // [in]
    void                 *pCmEventHandle;        // [in]
    uint32_t             uiHints;               // [in]
    uint32_t             iEventIndex;           // [out] index of pCmEventHandle in m_EventArray
    int32_t              iReturnValue;          // [out]
}CM_ENQUEUEHINTS_PARAM, *PCM_ENQUEUEHINTS_PARAM;

typedef struct _CM_DESTROYEVENT_PARAM
{
    void                *pCmQueueHandle;         // [in]
    void                *pCmEventHandle;         // [in]
    int32_t             iReturnValue;           // [out]
}CM_DESTROYEVENT_PARAM, *PCM_DESTROYEVENT_PARAM;

typedef struct _CM_CREATETHREADSPACE_PARAM
{
    uint32_t            TsWidth;                // [in]
    uint32_t            TsHeight;               // [in]
    void                *pCmTsHandle;            // [out]
    uint32_t            indexInTSArray;         // [out]
    int32_t             iReturnValue;           // [out]               
}CM_CREATETHREADSPACE_PARAM, *PCM_CREATETHREADSPACE_PARAM;

typedef struct _CM_DESTROYTHREADSPACE_PARAM
{
    void                *pCmTsHandle;            // [in]
    int32_t             iReturnValue;           // [out]
}CM_DESTROYTHREADSPACE_PARAM, *PCM_DESTROYTHREADSPACE_PARAM;

typedef struct _CM_DESTROYVMESURFACE_PARAM
{
    void                    *pCmVmeSurfIndexHandle;    // [in]
    int32_t                 iReturnValue;             // [out]
}CM_DESTROYVMESURFACE_PARAM, *PCM_DESTROYVMESURFACE_PARAM;

typedef struct _CM_CONFIGVMESURFACEDIMENSION_PARAM
{
    void                        *pCmVmeSurfHandle;    // [in]
    CM_VME_SURFACE_STATE_PARAM  *pSurfDimPara;        // [in]
    int32_t                     iReturnValue;         // [out]
}CM_CONFIGVMESURFACEDIMENSION_PARAM, *PCM_CONFIGVMESURFACEDIMENSION_PARAM;

typedef struct _CM_CREATEVMESURFACE_PARAM
{
    void                    *pCmCurSurfHandle;         // [in]
    void                    *pCmForwardSurfArray;      // [in]
    void                    *pCmBackwardSurfArray;     // [in]
    uint32_t                iForwardSurfCount;        // [in]
    uint32_t                iBackwardSurfCount;       // [in]
    void                    *pCmVmeSurfIndexHandle;    // [out]
    int32_t                 iReturnValue;             // [out]
}CM_CREATEVMESURFACE_PARAM, *PCM_CREATEVMESURFACE_PARAM;

typedef struct _CM_CREATESAMPLER_PARAM
{
    CM_SAMPLER_STATE        SampleState;                // [in]
    void                    *pCmSamplerHandle;           // [out]
    void                    *pCmSamplerIndexHandle;      // [out]
    int32_t                 iReturnValue;               // [out]
}CM_CREATESAMPLER_PARAM, *PCM_CREATESAMPLER_PARAM;

typedef struct _CM_CREATESAMPLER_PARAM_EX
{
    CM_SAMPLER_STATE_EX     SampleState;                // [in]
    void                    *pCmSamplerHandle;           // [out]
    void                    *pCmSamplerIndexHandle;      // [out]
    int32_t                 iReturnValue;               // [out]
}CM_CREATESAMPLER_PARAM_EX, *PCM_CREATESAMPLER_PARAM_EX;

typedef struct _CM_DESTROYSAMPLER_PARAM
{
    void                    *pCmSamplerHandle;          // [in]
    int32_t                 iReturnValue;              // [out]
}CM_DESTROYSAMPLER_PARAM, *PCM_DESTROYSAMPLER_PARAM;

typedef struct _CM_ENQUEUEGROUP_PARAM
{
    void                *pCmQueueHandle;         // [in]
    void                *pCmTaskHandle;          // [in]
    void                *pCmTGrpSpaceHandle;     // [in]
    void                *pCmEventHandle;         // [out]
    uint32_t            iEventIndex;            // [out] index of pCmEventHandle in m_EventArray
    int32_t             iReturnValue;           // [out]               
}CM_ENQUEUEGROUP_PARAM, *PCM_ENQUEUEGROUP_PARAM;

typedef struct _CM_CREATETGROUPSPACE_PARAM
{
    uint32_t                thrdSpaceWidth;              // [in]
    uint32_t                thrdSpaceHeight;             // [in]
    uint32_t                thrdSpaceDepth;              // [in]
    uint32_t                grpSpaceWidth;               // [in]
    uint32_t                grpSpaceHeight;              // [in]
    uint32_t                grpSpaceDepth;               // [in]
    void                    *pCmGrpSpaceHandle;           // [out]
    uint32_t                iTGSIndex;                   // [out] 
    int32_t                 iReturnValue;                // [out]
}CM_CREATETGROUPSPACE_PARAM, *PCM_CREATETGROUPSPACE_PARAM;

typedef struct _CM_DESTROYTGROPUSPACE_PARAM
{
    void                    *pCmGrpSpaceHandle;          // [in]
    int32_t                 iReturnValue;               // [out]
}CM_DESTROYTGROPUSPACE_PARAM, *PCM_DESTROYTGROPUSPACE_PARAM;

typedef struct _CM_GETSURFACE2DINFO_PARAM
{
    uint32_t                                iWidth;                 // [in]         Surface Width
    uint32_t                                iHeight;                // [in]         Surface Height
    CM_OSAL_SURFACE_FORMAT                  format;                 // [in]         Surface Format
    uint32_t                                iPitch;                 // [out]        Pitch
    uint32_t                                iPhysicalSize;          // [out]        Physical size
    uint32_t                                iReturnValue;           // [out]        Return value
} CM_GETSURFACE2DINFO_PARAM, *PCM_GETSURFACE2DINFO_PARAM;

typedef struct _CM_GETCAPS_PARAM
{
    CM_DEVICE_CAP_NAME              capName;                //[in]
    uint32_t                        capValueSize;           //[in]
    void                            *pCapValue;              //[in/out]
    uint32_t                        iReturnValue;           //[out]        Return value
}CM_GETCAPS_PARAM, *PCM_GETCAPS_PARAM;

//CM_ENQUEUE_GPUCOPY_PARAM version 2: two new fields are added
typedef struct _CM_ENQUEUE_GPUCOPY_PARAM
{
    void                    *pCmQueueHandle;         // [in] CmQueue pointer in CMRT@UMD
    void                    *pCmSurface2d;           // [in] CmSurface2d pointer in CMRT@UMD
    void                    *pSysMem;                // [in] pointer of system memory
    CM_GPUCOPY_DIRECTION    iCopyDir;               // [in] direction for GPUCopy: CM_FASTCOPY_GPU2CPU (0) or CM_FASTCOPY_CPU2GPU(1)
    uint32_t                iWidthStride;           // [in] width stride in byte for system memory, ZERO means no setting
    uint32_t                iHeightStride;          // [in] height stride in row for system memory, ZERO means no setting
    uint32_t                iOption;                // [in] option passed by user, only support CM_FASTCOPY_OPTION_NONBLOCKING(0) and CM_FASTCOPY_OPTION_BLOCKING(1)
    void                    *pCmEventHandle;         // [in/out] return CmDevice pointer in CMRT@UMD, nullptr if the input is CM_NO_EVENT
    uint32_t                iEventIndex;            // [out] index of Event in m_EventArray
    int32_t                 iReturnValue;           // [out] return value from CMRT@UMD              
} CM_ENQUEUE_GPUCOPY_PARAM, *PCM_ENQUEUE_GPUCOPY_PARAM;

typedef struct _CM_ENQUEUE_GPUCOPY_V2V_PARAM
{
    void                    *pCmQueueHandle;         // [in]
    void                    *pCmSrcSurface2d;        // [in]
    void                    *pCmDstSurface2d;        // [in]
    uint32_t                iOption;                 // [in]
    void                    *pCmEventHandle;         // [out]
    uint32_t                iEventIndex;             // [out] index of pCmEventHandle in m_EventArray
    int32_t                 iReturnValue;            // [out]               
}CM_ENQUEUE_GPUCOPY_V2V_PARAM, *PCM_ENQUEUE_GPUCOPY_V2V_PARAM;

typedef struct _CM_ENQUEUE_GPUCOPY_L2L_PARAM
{
    void                    *pCmQueueHandle;         // [in]
    void                    *pSrcSysMem;             // [in]
    void                    *pDstSysMem;             // [in]
    uint32_t                CopySize;                // [in]
    uint32_t                iOption;                 // [in]
    void                    *pCmEventHandle;         // [out]
    uint32_t                iEventIndex;             // [out] index of pCmEventHandle in m_EventArray
    int32_t                 iReturnValue;            // [out]               
}CM_ENQUEUE_GPUCOPY_L2L_PARAM, *PCM_ENQUEUE_GPUCOPY_L2L_PARAM;

typedef struct _CM_CREATE_SURFACE3D_PARAM
{
    uint32_t iWidth;                // [in] width of 3D  in pixel
    uint32_t iHeight;               // [in] height of 3D  in pixel
    uint32_t iDepth;                // [in] depth of 3D surface in pixel
    CM_OSAL_SURFACE_FORMAT Format;  // [in] 2D texture foramt in OS abstraction layer.
    void *pCmSurface3DHandle;       // [out] pointer of CmSurface3D used in driver
    int32_t iReturnValue;           // [out] the return value from driver
} CM_CREATE_SURFACE3D_PARAM, *PCM_CREATE_SURFACE3D_PARAM;

typedef struct _CM_DESTROY_SURFACE3D_PARAM
{
    void        *pCmSurface3DHandle;       // [in] pointer of CmSurface3D used in driver
    int32_t     iReturnValue;             // [out] the return value from driver
}CM_DESTROY_SURFACE3D_PARAM, *PCM_DESTROY_SURFACE3D_PARAM;

typedef struct _CM_CREATESAMPLER2D_PARAM
{
    void                *pCmSurface2DHandle;         // [in] pointer to CmSurface2D object used by CMRT@UMD
    void                *pSamplerSurfIndexHandle;    // [out] pointer of SurfaceIndex used in driver
    int32_t             iReturnValue;               // [out] the return value from CMRT@UMD
}CM_CREATESAMPLER2D_PARAM, *PCM_CREATESAMPLER2D_PARAM;

typedef struct _CM_CREATESAMPLER2DUP_PARAM
{
    void                *pCmSurface2DUPHandle;       // [in] pointer to CmSurface2DUP object used by CMRT@UMD
    void                *pSamplerSurfIndexHandle;    // [out] pointer of SurfaceIndex used in driver
    int32_t             iReturnValue;               // [out] the return value from CMRT@UMD
}CM_CREATESAMPLER2DUP_PARAM, *PCM_CREATESAMPLER2DUP_PARAM;

typedef struct _CM_CREATESAMPLER3D_PARAM
{
    void                *pCmSurface3DHandle;         // [in] pointer to CmSurface3D object used by CMRT@UMD
    void                *pSamplerSurfIndexHandle;    // [out] pointer of SurfaceIndex used in driver
    int32_t             iReturnValue;               // [out] the return value from CMRT@UMD
 }CM_CREATESAMPLER3D_PARAM, *PCM_CREATESAMPLER3D_PARAM;
    
typedef struct _CM_DESTROYSAMPLERSURF_PARAM
{
    void                *pSamplerSurfIndexHandle;    // [in] pointer of SamplerSurfaceIndex used in driver
    int32_t             iReturnValue;               // [out] the return value from CMRT@UMD
}CM_DESTROYSAMPLERSURF_PARAM, *PCM_DESTROYSAMPLERSURF_PARAM;

typedef struct _CM_DEVICE_SETCAP_PARAM
{
    CM_DEVICE_CAP_NAME  capName;                    // [in] Cap Type
    size_t              capValueSize;               // [in] Value Size
    void                *pCapValue;                  // [in] Pointer to value
    int32_t             iReturnValue;               // [out] the return value from CMRT@UMD
}CM_DEVICE_SETCAP_PARAM, *PCM_DEVICE_SETCAP_PARAM;

typedef struct _CM_DEVICE_SETSUGGESTEDL3_PARAM
{
    L3_SUGGEST_CONFIG   l3_s_c;                      // [in] Cap Type
    int32_t             iReturnValue;               // [out] the return value from CMRT@UMD
}CM_DEVICE_SETSUGGESTEDL3_PARAM, *PCM_DEVICE_SETSUGGESTEDL3_PARAM;

typedef struct _CM_CREATESAMPLER8x8_PARAM
{
    CM_SAMPLER_8X8_DESCR        Sample8x8Desc;                // [in]
    void                        *pCmSampler8x8Handle;          // [out]
    SamplerIndex*               pCmSamplerIndexHandle;        // [out]
    int32_t                     iReturnValue;                 // [out]
}CM_CREATESAMPLER8x8_PARAM, *PCM_CREATESAMPLER8x8_PARAM;

typedef struct _CM_DESTROYSAMPLER8x8_PARAM
{
    void                    *pCmSampler8x8Handle;          // [in]
    int32_t                 iReturnValue;                 // [out]
}CM_DESTROYSAMPLER8x8_PARAM, *PCM_DESTROYSAMPLER8x8_PARAM;

typedef struct _CM_CREATESAMPLER8x8SURF_PARAM
{
    void                        *pCmSurf2DHandle;              // [in]
    CM_SAMPLER8x8_SURFACE       CmSampler8x8Type;             // [in]
    CM_SURFACE_ADDRESS_CONTROL_MODE Sampler8x8Mode;           // [in]
    SurfaceIndex*               pCmSurfIndexHandle;           // [out]
    int32_t                     iReturnValue;                 // [out]
}CM_CREATESAMPLER8x8SURF_PARAM, *PCM_CREATESAMPLER8x8SURF_PARAM;

typedef struct _CM_CREATESAMPLER8x8SURFEX_PARAM
{
    void                        *pCmSurf2DHandle;              // [in]
    CM_SAMPLER8x8_SURFACE       CmSampler8x8Type;             // [in]
    CM_SURFACE_ADDRESS_CONTROL_MODE Sampler8x8Mode;           // [in]
    CM_FLAG*                    pFlag;                        // [in]
    SurfaceIndex*               pCmSurfIndexHandle;           // [out]
    int32_t                     iReturnValue;                 // [out]
}CM_CREATESAMPLER8x8SURFEX_PARAM, *PCM_CREATESAMPLER8x8SURFEX_PARAM;

typedef struct _CM_CREATESAMPLER2DEX_PARAM
{
    void                *pCmSurface2DHandle;                   // [in] 
    CM_FLAG*            pFlag;                                // [in]
    void                *pSamplerSurfIndexHandle;              // [out] 
    int32_t             iReturnValue;                         // [out] 
}CM_CREATESAMPLER2DEX_PARAM, *PCM_CREATESAMPLER2DEX_PARAM;

typedef struct _CM_DESTROYSAMPLER8x8SURF_PARAM
{
    SurfaceIndex*               pCmSurfIndexHandle;           // [in]
    int32_t                     iReturnValue;                 // [out]
}CM_DESTROYSAMPLER8x8SURF_PARAM, *PCM_DESTROYSAMPLER8x8SURF_PARAM;

typedef struct _CM_ENQUEUE_2DINIT_PARAM
{
    void                    *pCmQueueHandle;         // [in] handle of Queue
    void                    *pCmSurface2d;           // [in] handle of surface 2d
    uint32_t                dwInitValue;            // [in] init value  
    void                    *pCmEventHandle;         // [out] event's handle
    uint32_t                iEventIndex;            // [out] event's index
    int32_t                 iReturnValue;           // [out] return value               
}CM_ENQUEUE_2DINIT_PARAM, *PCM_ENQUEUE_2DINIT_PARAM;

typedef struct _CM_DEVICE_INIT_PRINT_BUFFER_PARAM
{
    uint32_t            dwPrintBufferSize;              //[in]  print buffer's size
    void                *pPrintBufferMem;                //[out] print buffer's memory
    int32_t             iReturnValue;                   //[out] return value
}CM_DEVICE_INIT_PRINT_BUFFER_PARAM, *PCM_DEVICE_INIT_PRINT_BUFFER_PARAM;

typedef struct _CM_CREATEVEBOX_PARAM
{
    void                    *pCmVeboxHandle;         // [out] CmVeboxG75's handle
    uint32_t                indexInVeboxArray;      // [out] index in m_VeboxArray
    int32_t                 iReturnValue;           // [out] return value               
}CM_CREATEVEBOX_PARAM, *PCM_CREATEVEBOX_PARAM;

typedef struct _CM_DESTROYVEBOX_PARAM
{
    void                    *pCmVeboxHandle;         // [IN] CmVeboxG75's handle
    int32_t                 iReturnValue;           // [out] return value               
}CM_DESTROYVEBOX_PARAM, *PCM_DESTROYVEBOX_PARAM;

typedef struct _CM_DEVICE_CREATE_SURF2D_ALIAS_PARAM
{
    void                    *pCmSurface2DHandle;      // [IN] pointer to CMSurface2D
    void                    *pSurfaceIndexHandle;     // [OUT] pointer of SurfaceIndex
    int32_t                 iReturnValue;            // [OUT] return value
} CM_DEVICE_CREATE_SURF2D_ALIAS_PARAM, *PCM_DEVICE_CREATE_SURF2D_ALIAS_PARAM;

typedef struct _CM_DEVICE_CREATE_BUFFER_ALIAS_PARAM
{
    void                    *pCmBufferHandle;        // [IN] pointer to CmBuffer object
    void                    *pSurfaceIndexHandle;    // [OUT] ponter of SurfaceIndex
    int32_t                 iReturnValue;           // [OUT] return value
} CM_DEVICE_CREATE_BUFFER_ALIAS_PARAM, *PCM_DEVICE_CREATE_BUFFER_ALIAS_PARAM;

typedef struct _CM_CLONE_KERNEL_PARAM
{
    void                    *pCmKernelHandleSrc;     // [IN] source kernel
    void                    *pCmKernelHandleDest;    // [OUT] dest kernel
    int32_t                 iReturnValue;           // [OUT] return value
}CM_CLONE_KERNEL_PARAM, *PCM_CLONE_KERNEL_PARAM;

typedef struct _CM_ENQUEUE_VEBOX_PARAM
{
    void                    *pCmQueueHandle;         // [IN] 
    void                    *pCmVeboxHandle;         // [IN] CmVeboxG75's handle
    void                    *pCmEventHandle;         // [out] event's handle
    uint32_t                iEventIndex;            // [out] event's index
    int32_t                 iReturnValue;           // [out] return value               
}CM_ENQUEUE_VEBOX_PARAM, *PCM_ENQUEUE_VEBOX_PARAM;

struct CM_GET_VISA_VERSION_PARAM
{
    uint32_t                  iMajorVersion;         // [OUT] the major version of jitter
    uint32_t                  iMinorVersion;         // [OUT] the minor version of jitter
    int32_t                   iReturnValue;          // [OUT] return value
};

//!
//! \brief    Creates a CmDevice from a MOS context.
//! \details  If an existing CmDevice has already associated to the MOS context,
//!           the existing CmDevice will be returned. Otherwise, a new CmDevice
//!           instance will be created and associatied with that MOS context.
//! \param    pMosContext
//!           [in] pointer to MOS conetext.
//! \param    pDevice
//!           [in/out] reference to the pointer to the CmDevice.
//! \param    devCreateOption
//!           [in] option to customize CmDevice.
//! \retval   CM_SUCCESS if the CmDevice is successfully created.
//! \retval   CM_NULL_POINTER if pMosContext is null.
//! \retval   CM_FAILURE otherwise.
//!
CM_RT_API int32_t CreateCmDevice(MOS_CONTEXT *pMosContext, CmDevice* &pDevice, uint32_t devCreateOption);

//!
//! \brief    Destroys the CmDevice associated with MOS context. 
//! \details  This function also destroys surfaces, kernels, programs, samplers,
//!           threadspaces, tasks and the queues that were created using this
//!           device instance but haven’t explicitly been destroyed by calling
//!           respective destroy functions. 
//! \param    pMosContext
//!           [in] pointer to MOS conetext.
//! \retval   CM_SUCCESS if CmDevice is successfully destroyed.
//! \retval   CM_NULL_POINTER if MOS context is null.
//! \retval   CM_FAILURE otherwise.
//!
CM_RT_API int32_t DestroyCmDevice(MOS_CONTEXT *pMosContext);

//!
//! \brief      Returns the corresponding CM_RETURN_CODE error string.
//! \param      [in] errCode
//!             CM error code.
//! \return     Corresponding error string if valid Code. \n
//!             "Internal Error" if invalid.
//!
CM_RT_API const char* GetCmErrorString(int errCode);
//////////////////////////////////////////////////////////////////////////////////////
// Thin CMRT definition -- END
//////////////////////////////////////////////////////////////////////////////////////


#endif
