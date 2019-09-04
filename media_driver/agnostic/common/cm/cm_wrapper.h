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
//! \brief     Contains declarations of various OS-agnostic data structures and
//!            functions for executing commands from cmrtlib.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMWRAPPER_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMWRAPPER_H_

#include "cm_wrapper_os.h"
#include "cm_device.h"

#define CM_BOUNDARY_PIXEL_MODE GFX3DSTATE_MEDIA_BOUNDARY_PIXEL_MODE

#define CM_SURFACE_2D(pSurf) static_cast<CmSurface2DRT*>((CmSurface2D *)(pSurf))

typedef struct _CM_DESTROYCMDEVICE_PARAM
{
    void        *deviceHandle;        // [in/out] pointer to CmDevice object
    int32_t     returnValue;           // [out] the return value from CMRT@UMD
}CM_DESTROYCMDEVICE_PARAM, *PCM_DESTROYCMDEVICE_PARAM;

typedef struct _CM_DESTROYBUFFER_PARAM
{
    void        *bufferHandle;       // [in/out] pointer to CmBuffer object
    int32_t     returnValue;           // [out] the return value from CMRT@UMD
}CM_DESTROYBUFFER_PARAM, *PCM_DESTROYBUFFER_PARAM;

typedef struct _CM_DESTROYSURFACE2D_PARAM
{
    void        *surface2DHandle;         // [in/out] pointer to CmSurface2D object
    int32_t     returnValue;               // [out] the return value from CMRT@UMD
}CM_DESTROYSURFACE2D_PARAM, *PCM_DESTROYSURFACE2D_PARAM;

typedef struct _CM_CREATESURFACE2DUP_PARAM
{
    uint32_t width;                // [in] width of 2D texture in pixel
    uint32_t height;               // [in] height of 2D texture in pixel
    CM_OSAL_SURFACE_FORMAT format;  // [in] 2D texture foramt in OS layer.
    void *sysMem;                  // [in] Pointer to system memory
    void *surface2DUPHandle;     // [out] pointer of CmSurface2D used in driver
    int32_t returnValue;           // [out] the return value from driver
}CM_CREATESURFACE2DUP_PARAM, *PCM_CREATESURFACE2DUP_PARAM;

typedef struct _CM_DESTROYSURFACE2DUP_PARAM
{
    void        *surface2DUPHandle;         // [in/out] pointer to CmSurface2D object
    int32_t     returnValue;               // [out] the return value from CMRT@UMD
}CM_DESTROYSURFACE2DUP_PARAM, *PCM_DESTROYSURFACE2DUP_PARAM;

typedef struct _CM_LOADPROGRAM_PARAM
{
    void                *cisaCode;              // [in] pointer to the CISA code buffer
    uint32_t            cisaCodeSize;         // [in] size of CISA code
    char*               options;                // [in] additonal options for LoadProgram
    void                *programHandle;       // [out] pointer to CmProgram object used by CMRT@UMD
    uint32_t            indexInArray;           // [out] index in m_ProgramArray of CMRT@UMD
    int32_t             returnValue;           // [out] the return value from CMRT@UMD
}CM_LOADPROGRAM_PARAM, *PCM_LOADPROGRAM_PARAM;

typedef struct _CM_DESTROYPROGRAM_PARAM
{
    void                *programHandle;       // [IN] pointer to CmProgram object used by CMRT@UMD
    int32_t             returnValue;           // [out] the return value from CMRT@UMD
}CM_DESTROYPROGRAM_PARAM, *PCM_DESTROYPROGRAM_PARAM;

typedef struct _CM_CREATEKERNEL_PARAM
{
    void                *programHandle;       // [in] pointer to CmProgram used in driver
    char*               kernelName;            // [in] pointer to the kernel name string
    char*               options;               // [in] pointer to the kernel creation options
    void                *kernelHandle;        // [out] pointer to new created CmKernel used in driver
    uint32_t            indexKernelArray;       // [out] index in m_KernelArray
    int32_t             returnValue;           // [out] the return value from driver
}CM_CREATEKERNEL_PARAM, *PCM_CREATEKERNEL_PARAM;

typedef struct _CM_DESTROYKERNEL_PARAM
{
    void                *kernelHandle;        // [in/out] pointer to new created CmKernel used in driver
    int32_t             returnValue;           // [out] the return value from driver
}CM_DESTROYKERNEL_PARAM, *PCM_DESTROYKERNEL_PARAM;

typedef struct _CM_SETSURFACEMEMORYOBJECTCTRL_PARAM
{
    void                    *surfaceHandle;       // [in]
    MEMORY_OBJECT_CONTROL   memCtrl;                // [in]
    MEMORY_TYPE             memType;                // [in]
    uint32_t                age;                  // [in]
    int32_t                 returnValue;           // [out]
}CM_SETSURFACEMEMORYOBJECTCTRL_PARAM, *PCM_SETSURFACEMEMORYOBJECTCTRL_PARAM;

typedef struct _CM_CREATETASK_PARAM
{
    void                *taskHandle;          // [out] pointer to new created CmTask used in driver
    uint32_t            taskIndex;             // [out] index of task i
    int32_t             returnValue;           // [out] the return value from driver
}CM_CREATETASK_PARAM, *PCM_CREATETASK_PARAM;

typedef struct _CM_DESTROYTASK_PARAM
{
    void                *taskHandle;          // [in/out] pointer to CmTask used in driver
    int32_t             returnValue;           // [out] the return value from driver
}CM_DESTROYTASK_PARAM, *PCM_DESTROYTASK_PARAM;

struct CM_CREATEQUEUE_PARAM
{
    CM_QUEUE_CREATE_OPTION  createOption;        // [in/out]
    void                   *queueHandle;         // [out]
    int32_t                 returnValue;         // [out]
};

typedef struct _CM_ENQUEUE_PARAM
{
    void                *queueHandle;         // [in]
    void                *taskHandle;          // [in]
    void                *threadSpaceHandle;   // [in]
    void                *eventHandle;         // [out]
    uint32_t            eventIndex;            // [out] index of pCmEventHandle in m_EventArray
    int32_t             returnValue;           // [out]
}CM_ENQUEUE_PARAM, *PCM_ENQUEUE_PARAM;

typedef struct _CM_ENQUEUEHINTS_PARAM
{
    void                 *queueHandle;        // [in]
    void                 *taskHandle;         // [in]
    void                 *eventHandle;        // [in]
    uint32_t             hints;               // [in]
    uint32_t             eventIndex;           // [out] index of pCmEventHandle in m_EventArray
    int32_t              returnValue;          // [out]
}CM_ENQUEUEHINTS_PARAM, *PCM_ENQUEUEHINTS_PARAM;

typedef struct _CM_DESTROYEVENT_PARAM
{
    void                *queueHandle;         // [in]
    void                *eventHandle;         // [in]
    int32_t             returnValue;           // [out]
}CM_DESTROYEVENT_PARAM, *PCM_DESTROYEVENT_PARAM;

typedef struct _CM_CREATETHREADSPACE_PARAM
{
    uint32_t            threadSpaceWidth;                // [in]
    uint32_t            threadSpaceHeight;               // [in]
    void                *threadSpaceHandle;            // [out]
    uint32_t            indexInTSArray;         // [out]
    int32_t             returnValue;           // [out]
}CM_CREATETHREADSPACE_PARAM, *PCM_CREATETHREADSPACE_PARAM;

typedef struct _CM_DESTROYTHREADSPACE_PARAM
{
    void                *threadSpaceHandle;            // [in]
    int32_t             returnValue;           // [out]
}CM_DESTROYTHREADSPACE_PARAM, *PCM_DESTROYTHREADSPACE_PARAM;

typedef struct _CM_DESTROYVMESURFACE_PARAM
{
    void                    *vmeSurfIndexHandle;    // [in]
    int32_t                 returnValue;             // [out]
}CM_DESTROYVMESURFACE_PARAM, *PCM_DESTROYVMESURFACE_PARAM;

typedef struct _CM_CONFIGVMESURFACEDIMENSION_PARAM
{
    void                        *vmeSurfHandle;    // [in]
    CM_VME_SURFACE_STATE_PARAM  *surfDimensionPara;        // [in]
    int32_t                     returnValue;         // [out]
}CM_CONFIGVMESURFACEDIMENSION_PARAM, *PCM_CONFIGVMESURFACEDIMENSION_PARAM;

typedef struct _CM_CREATEVMESURFACE_PARAM
{
    void                    *curSurfHandle;         // [in]
    void                    *forwardSurfArray;      // [in]
    void                    *backwardSurfArray;     // [in]
    uint32_t                forwardSurfCount;        // [in]
    uint32_t                backwardSurfCount;       // [in]
    void                    *vmeSurfIndexHandle;    // [out]
    int32_t                 returnValue;             // [out]
}CM_CREATEVMESURFACE_PARAM, *PCM_CREATEVMESURFACE_PARAM;

typedef struct _CM_CREATESAMPLER_PARAM
{
    CM_SAMPLER_STATE        sampleState;                // [in]
    void                    *samplerHandle;           // [out]
    void                    *samplerIndexHandle;      // [out]
    int32_t                 returnValue;               // [out]
}CM_CREATESAMPLER_PARAM, *PCM_CREATESAMPLER_PARAM;

typedef struct _CM_CREATESAMPLER_PARAM_EX
{
    CM_SAMPLER_STATE_EX     sampleState;                // [in]
    void                    *samplerHandle;           // [out]
    void                    *samplerIndexHandle;      // [out]
    int32_t                 returnValue;               // [out]
}CM_CREATESAMPLER_PARAM_EX, *PCM_CREATESAMPLER_PARAM_EX;

typedef struct _CM_DESTROYSAMPLER_PARAM
{
    void                    *samplerHandle;          // [in]
    int32_t                 returnValue;              // [out]
}CM_DESTROYSAMPLER_PARAM, *PCM_DESTROYSAMPLER_PARAM;

typedef struct _CM_ENQUEUEGROUP_PARAM
{
    void                *queueHandle;         // [in]
    void                *taskHandle;          // [in]
    void                *threadGroupSpaceHandle;     // [in]
    void                *eventHandle;         // [out]
    uint32_t            eventIndex;            // [out] index of pCmEventHandle in m_EventArray
    int32_t             returnValue;           // [out]
}CM_ENQUEUEGROUP_PARAM, *PCM_ENQUEUEGROUP_PARAM;

typedef struct _CM_CREATETGROUPSPACE_PARAM
{
    uint32_t                thrdSpaceWidth;              // [in]
    uint32_t                thrdSpaceHeight;             // [in]
    uint32_t                thrdSpaceDepth;              // [in]
    uint32_t                grpSpaceWidth;               // [in]
    uint32_t                grpSpaceHeight;              // [in]
    uint32_t                grpSpaceDepth;               // [in]
    void                    *groupSpaceHandle;           // [out]
    uint32_t                threadGroupSpaceIndex;                   // [out]
    int32_t                 returnValue;                // [out]
}CM_CREATETGROUPSPACE_PARAM, *PCM_CREATETGROUPSPACE_PARAM;

typedef struct _CM_DESTROYTGROPUSPACE_PARAM
{
    void                    *groupSpaceHandle;          // [in]
    int32_t                 returnValue;               // [out]
}CM_DESTROYTGROPUSPACE_PARAM, *PCM_DESTROYTGROPUSPACE_PARAM;

typedef struct _CM_GETSURFACE2DINFO_PARAM
{
    uint32_t                                width;                 // [in]         Surface Width
    uint32_t                                height;                // [in]         Surface Height
    CM_OSAL_SURFACE_FORMAT                  format;                 // [in]         Surface Format
    uint32_t                                pitch;                 // [out]        Pitch
    uint32_t                                physicalSize;          // [out]        Physical size
    uint32_t                                returnValue;           // [out]        Return value
} CM_GETSURFACE2DINFO_PARAM, *PCM_GETSURFACE2DINFO_PARAM;

typedef struct _CM_GETCAPS_PARAM
{
    CM_DEVICE_CAP_NAME              capName;                //[in]
    uint32_t                        capValueSize;           //[in]
    void                            *capValue;              //[in/out]
    uint32_t                        returnValue;           //[out]        Return value
}CM_GETCAPS_PARAM, *PCM_GETCAPS_PARAM;

typedef struct _CM_ENQUEUE_GPUCOPY_V2V_PARAM
{
    void                    *queueHandle;         // [in]
    void                    *srcSurface2d;        // [in]
    void                    *dstSurface2d;        // [in]
    uint32_t                option;                 // [in]
    void                    *eventHandle;         // [out]
    uint32_t                eventIndex;             // [out] index of pCmEventHandle in m_EventArray
    int32_t                 returnValue;            // [out]
}CM_ENQUEUE_GPUCOPY_V2V_PARAM, *PCM_ENQUEUE_GPUCOPY_V2V_PARAM;

typedef struct _CM_ENQUEUE_GPUCOPY_L2L_PARAM
{
    void                    *queueHandle;         // [in]
    void                    *srcSysMem;             // [in]
    void                    *dstSysMem;             // [in]
    uint32_t                copySize;                // [in]
    uint32_t                option;                 // [in]
    void                    *eventHandle;         // [out]
    uint32_t                eventIndex;             // [out] index of pCmEventHandle in m_EventArray
    int32_t                 returnValue;            // [out]
}CM_ENQUEUE_GPUCOPY_L2L_PARAM, *PCM_ENQUEUE_GPUCOPY_L2L_PARAM;

typedef struct _CM_CREATE_SURFACE3D_PARAM
{
    uint32_t width;                // [in] width of 3D  in pixel
    uint32_t height;               // [in] height of 3D  in pixel
    uint32_t depth;                // [in] depth of 3D surface in pixel
    CM_OSAL_SURFACE_FORMAT format;  // [in] 2D texture foramt in OS abstraction layer.
    void *surface3DHandle;       // [out] pointer of CmSurface3D used in driver
    int32_t returnValue;           // [out] the return value from driver
} CM_CREATE_SURFACE3D_PARAM, *PCM_CREATE_SURFACE3D_PARAM;

typedef struct _CM_DESTROY_SURFACE3D_PARAM
{
    void        *surface3DHandle;       // [in] pointer of CmSurface3D used in driver
    int32_t     returnValue;             // [out] the return value from driver
}CM_DESTROY_SURFACE3D_PARAM, *PCM_DESTROY_SURFACE3D_PARAM;

typedef struct _CM_CREATESAMPLER2D_PARAM
{
    void                *surface2DHandle;         // [in] pointer to CmSurface2D object used by CMRT@UMD
    void                *samplerSurfIndexHandle;    // [out] pointer of SurfaceIndex used in driver
    int32_t             returnValue;               // [out] the return value from CMRT@UMD
}CM_CREATESAMPLER2D_PARAM, *PCM_CREATESAMPLER2D_PARAM;

typedef struct _CM_CREATESAMPLER2DUP_PARAM
{
    void                *surface2DUPHandle;       // [in] pointer to CmSurface2DUP object used by CMRT@UMD
    void                *samplerSurfIndexHandle;    // [out] pointer of SurfaceIndex used in driver
    int32_t             returnValue;               // [out] the return value from CMRT@UMD
}CM_CREATESAMPLER2DUP_PARAM, *PCM_CREATESAMPLER2DUP_PARAM;

typedef struct _CM_CREATESAMPLER3D_PARAM
{
    void                *surface3DHandle;         // [in] pointer to CmSurface3D object used by CMRT@UMD
    void                *samplerSurfIndexHandle;    // [out] pointer of SurfaceIndex used in driver
    int32_t             returnValue;               // [out] the return value from CMRT@UMD
 }CM_CREATESAMPLER3D_PARAM, *PCM_CREATESAMPLER3D_PARAM;

typedef struct _CM_DESTROYSAMPLERSURF_PARAM
{
    void                *samplerSurfIndexHandle;    // [in] pointer of SamplerSurfaceIndex used in driver
    int32_t             returnValue;               // [out] the return value from CMRT@UMD
}CM_DESTROYSAMPLERSURF_PARAM, *PCM_DESTROYSAMPLERSURF_PARAM;

typedef struct _CM_DEVICE_SETCAP_PARAM
{
    CM_DEVICE_CAP_NAME  capName;                    // [in] Cap Type
    size_t              capValueSize;               // [in] Value Size
    void                *capValue;                  // [in] Pointer to value
    int32_t             returnValue;               // [out] the return value from CMRT@UMD
}CM_DEVICE_SETCAP_PARAM, *PCM_DEVICE_SETCAP_PARAM;

typedef struct _CM_DEVICE_SETSUGGESTEDL3_PARAM
{
    L3_SUGGEST_CONFIG   l3SuggestConfig;                      // [in] Cap Type
    int32_t             returnValue;               // [out] the return value from CMRT@UMD
}CM_DEVICE_SETSUGGESTEDL3_PARAM, *PCM_DEVICE_SETSUGGESTEDL3_PARAM;

using CMRT_UMD::SamplerIndex;
typedef struct _CM_CREATESAMPLER8x8_PARAM
{
    CM_SAMPLER_8X8_DESCR        sample8x8Desc;                // [in]
    void                        *sampler8x8Handle;          // [out]
    SamplerIndex*               samplerIndexHandle;        // [out]
    int32_t                     returnValue;                 // [out]
}CM_CREATESAMPLER8x8_PARAM, *PCM_CREATESAMPLER8x8_PARAM;

typedef struct _CM_DESTROYSAMPLER8x8_PARAM
{
    void                    *sampler8x8Handle;          // [in]
    int32_t                 returnValue;                 // [out]
}CM_DESTROYSAMPLER8x8_PARAM, *PCM_DESTROYSAMPLER8x8_PARAM;

using CMRT_UMD::SurfaceIndex;
typedef struct _CM_CREATESAMPLER8x8SURF_PARAM
{
    void                        *surf2DHandle;              // [in]
    CM_SAMPLER8x8_SURFACE       sampler8x8Type;             // [in]
    CM_SURFACE_ADDRESS_CONTROL_MODE sampler8x8Mode;           // [in]
    SurfaceIndex*               surfIndexHandle;           // [out]
    int32_t                     returnValue;                 // [out]
}CM_CREATESAMPLER8x8SURF_PARAM, *PCM_CREATESAMPLER8x8SURF_PARAM;

typedef struct _CM_CREATESAMPLER8x8SURFEX_PARAM
{
    void                        *surf2DHandle;              // [in]
    CM_SAMPLER8x8_SURFACE       sampler8x8Type;             // [in]
    CM_SURFACE_ADDRESS_CONTROL_MODE sampler8x8Mode;           // [in]
    CM_FLAG*                    flag;                        // [in]
    SurfaceIndex*               surfIndexHandle;           // [out]
    int32_t                     returnValue;                 // [out]
}CM_CREATESAMPLER8x8SURFEX_PARAM, *PCM_CREATESAMPLER8x8SURFEX_PARAM;

typedef struct _CM_CREATESAMPLER2DEX_PARAM
{
    void                *surface2DHandle;                   // [in]
    CM_FLAG*            flag;                                // [in]
    void                *samplerSurfIndexHandle;              // [out]
    int32_t             returnValue;                         // [out]
}CM_CREATESAMPLER2DEX_PARAM, *PCM_CREATESAMPLER2DEX_PARAM;

typedef struct _CM_DESTROYSAMPLER8x8SURF_PARAM
{
    SurfaceIndex*               surfIndexHandle;           // [in]
    int32_t                     returnValue;                 // [out]
}CM_DESTROYSAMPLER8x8SURF_PARAM, *PCM_DESTROYSAMPLER8x8SURF_PARAM;

typedef struct _CM_ENQUEUE_2DINIT_PARAM
{
    void                    *queueHandle;         // [in] handle of Queue
    void                    *surface2d;           // [in] handle of surface 2d
    uint32_t                initValue;            // [in] init value
    void                    *eventHandle;         // [out] event's handle
    uint32_t                eventIndex;            // [out] event's index
    int32_t                 returnValue;           // [out] return value
}CM_ENQUEUE_2DINIT_PARAM, *PCM_ENQUEUE_2DINIT_PARAM;

typedef struct _CM_DEVICE_INIT_PRINT_BUFFER_PARAM
{
    uint32_t            printBufferSize;              //[in]  print buffer's size
    void                *printBufferMem;                //[out] print buffer's memory
    int32_t             returnValue;                   //[out] return value
}CM_DEVICE_INIT_PRINT_BUFFER_PARAM, *PCM_DEVICE_INIT_PRINT_BUFFER_PARAM;

typedef struct _CM_DEVICE_FLUSH_PRINT_BUFFER_PARAM
{
    const char          *fileName;                     //[in] target file name
    int32_t             returnValue;                   //[out] return value
}CM_DEVICE_FLUSH_PRINT_BUFFER_PARAM, *PCM_DEVICE_FLUSH_PRINT_BUFFER_PARAM;

typedef struct _CM_CREATEVEBOX_PARAM
{
    void                    *veboxHandle;         // [out] CmVeboxG75's handle
    uint32_t                indexInVeboxArray;      // [out] index in m_VeboxArray
    int32_t                 returnValue;           // [out] return value
}CM_CREATEVEBOX_PARAM, *PCM_CREATEVEBOX_PARAM;

typedef struct _CM_DESTROYVEBOX_PARAM
{
    void                    *veboxHandle;         // [IN] CmVeboxG75's handle
    int32_t                 returnValue;           // [out] return value
}CM_DESTROYVEBOX_PARAM, *PCM_DESTROYVEBOX_PARAM;

typedef struct _CM_DEVICE_CREATE_SURF2D_ALIAS_PARAM
{
    void                    *surface2DHandle;      // [IN] pointer to CMSurface2D
    void                    *surfaceIndexHandle;     // [OUT] pointer of SurfaceIndex
    int32_t                 returnValue;            // [OUT] return value
} CM_DEVICE_CREATE_SURF2D_ALIAS_PARAM, *PCM_DEVICE_CREATE_SURF2D_ALIAS_PARAM;

typedef struct _CM_DEVICE_CREATE_BUFFER_ALIAS_PARAM
{
    void                    *bufferHandle;        // [IN] pointer to CmBuffer object
    void                    *surfaceIndexHandle;    // [OUT] ponter of SurfaceIndex
    int32_t                 returnValue;           // [OUT] return value
} CM_DEVICE_CREATE_BUFFER_ALIAS_PARAM, *PCM_DEVICE_CREATE_BUFFER_ALIAS_PARAM;

typedef struct _CM_CLONE_KERNEL_PARAM
{
    void                    *kernelHandleSrc;     // [IN] source kernel
    void                    *kernelHandleDest;    // [OUT] dest kernel
    int32_t                 returnValue;           // [OUT] return value
}CM_CLONE_KERNEL_PARAM, *PCM_CLONE_KERNEL_PARAM;

typedef struct _CM_ENQUEUE_VEBOX_PARAM
{
    void                    *queueHandle;         // [IN]
    void                    *veboxHandle;         // [IN] CmVeboxG75's handle
    void                    *eventHandle;         // [out] event's handle
    uint32_t                eventIndex;            // [out] event's index
    int32_t                 returnValue;           // [out] return value
}CM_ENQUEUE_VEBOX_PARAM, *PCM_ENQUEUE_VEBOX_PARAM;

struct CM_GET_VISA_VERSION_PARAM
{
    uint32_t                  majorVersion;         // [OUT] the major version of jitter
    uint32_t                  minorVersion;         // [OUT] the minor version of jitter
    int32_t                   returnValue;          // [OUT] return value
};

//*-----------------------------------------------------------------------------
//| CM extension Function Codes
//*-----------------------------------------------------------------------------
enum CM_FUNCTION_ID
{
    CM_FN_RT_ULT      = 0x900,
    CM_FN_RT_ULT_INFO = 0x902,

    CM_FN_CREATECMDEVICE  = 0x1000,
    CM_FN_DESTROYCMDEVICE = 0x1001,

    CM_FN_CMDEVICE_CREATEBUFFER             = 0x1100,
    CM_FN_CMDEVICE_DESTROYBUFFER            = 0x1101,
    CM_FN_CMDEVICE_CREATEBUFFERUP           = 0x1102,
    CM_FN_CMDEVICE_DESTROYBUFFERUP          = 0x1103,
    CM_FN_CMDEVICE_CREATESURFACE2D          = 0x1104,
    CM_FN_CMDEVICE_DESTROYSURFACE2D         = 0x1105,
    CM_FN_CMDEVICE_CREATESURFACE2DUP        = 0x1106,
    CM_FN_CMDEVICE_DESTROYSURFACE2DUP       = 0x1107,
    CM_FN_CMDEVICE_GETSURFACE2DINFO         = 0x1108,
    CM_FN_CMDEVICE_CREATESURFACE3D          = 0x1109,
    CM_FN_CMDEVICE_DESTROYSURFACE3D         = 0x110A,
    CM_FN_CMDEVICE_CREATEQUEUE              = 0x110B,
    CM_FN_CMDEVICE_LOADPROGRAM              = 0x110C,
    CM_FN_CMDEVICE_DESTROYPROGRAM           = 0x110D,
    CM_FN_CMDEVICE_CREATEKERNEL             = 0x110E,
    CM_FN_CMDEVICE_DESTROYKERNEL            = 0x110F,
    CM_FN_CMDEVICE_CREATETASK               = 0x1110,
    CM_FN_CMDEVICE_DESTROYTASK              = 0x1111,
    CM_FN_CMDEVICE_GETCAPS                  = 0x1112,
    CM_FN_CMDEVICE_SETCAPS                  = 0x1113,
    CM_FN_CMDEVICE_CREATETHREADSPACE        = 0x1114,
    CM_FN_CMDEVICE_DESTROYTHREADSPACE       = 0x1115,
    CM_FN_CMDEVICE_CREATETHREADGROUPSPACE   = 0x1116,
    CM_FN_CMDEVICE_DESTROYTHREADGROUPSPACE  = 0x1117,
    CM_FN_CMDEVICE_SETL3CONFIG              = 0x1118,
    CM_FN_CMDEVICE_SETSUGGESTEDL3CONFIG     = 0x1119,
    CM_FN_CMDEVICE_CREATESAMPLER            = 0x111A,
    CM_FN_CMDEVICE_DESTROYSAMPLER           = 0x111B,
    CM_FN_CMDEVICE_CREATESAMPLER8X8         = 0x111C,
    CM_FN_CMDEVICE_DESTROYSAMPLER8X8        = 0x111D,
    CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE  = 0x111E,
    CM_FN_CMDEVICE_DESTROYSAMPLER8X8SURFACE = 0x111F,
    CM_FN_CMDEVICE_DESTROYVMESURFACE        = 0x1123,
    CM_FN_CMDEVICE_CREATEVMESURFACEG7_5     = 0x1124,
    CM_FN_CMDEVICE_DESTROYVMESURFACEG7_5    = 0x1125,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D   = 0x1126,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE3D   = 0x1127,
    CM_FN_CMDEVICE_DESTROYSAMPLERSURFACE    = 0x1128,
    CM_FN_CMDEVICE_ENABLE_GTPIN             = 0X112A,
    CM_FN_CMDEVICE_INIT_PRINT_BUFFER        = 0x112C,
    CM_FN_CMDEVICE_CREATEVEBOX              = 0x112D,
    CM_FN_CMDEVICE_DESTROYVEBOX             = 0x112E,
    CM_FN_CMDEVICE_CREATEBUFFERSVM          = 0x1131,
    CM_FN_CMDEVICE_DESTROYBUFFERSVM         = 0x1132,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE2DUP = 0x1133,
    CM_FN_CMDEVICE_REGISTER_GTPIN_MARKERS   = 0x1136,
    CM_FN_CMDEVICE_CLONEKERNEL              = 0x1137,
    CM_FN_CMDEVICE_CREATESURFACE2D_ALIAS    = 0x1138,
    CM_FN_CMDEVICE_CREATESAMPLER_EX         = 0x1139,

    CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE_EX = 0x113A,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D_EX  = 0x113B,
    CM_FN_CMDEVICE_CREATESURFACE2D_EX         = 0x113C,
    CM_FN_CMDEVICE_CREATEBUFFER_ALIAS         = 0x113D,
    CM_FN_CMDEVICE_CONFIGVMESURFACEDIMENSION  = 0x113E,
    CM_FN_CMDEVICE_CREATEHEVCVMESURFACEG10    = 0x113F,
    CM_FN_CMDEVICE_GETVISAVERSION             = 0x1140,
    CM_FN_CMDEVICE_CREATEQUEUEEX              = 0x1141,
    CM_FN_CMDEVICE_FLUSH_PRINT_BUFFER         = 0x1142,
    CM_FN_CMDEVICE_DESTROYBUFFERSTATELESS     = 0x1143,

    CM_FN_CMQUEUE_ENQUEUE           = 0x1500,
    CM_FN_CMQUEUE_DESTROYEVENT      = 0x1501,
    CM_FN_CMQUEUE_ENQUEUECOPY       = 0x1502,
    CM_FN_CMQUEUE_ENQUEUEWITHGROUP  = 0x1504,
    CM_FN_CMQUEUE_ENQUEUESURF2DINIT = 0x1505,
    CM_FN_CMQUEUE_ENQUEUECOPY_V2V   = 0x1506,
    CM_FN_CMQUEUE_ENQUEUECOPY_L2L   = 0x1507,
    CM_FN_CMQUEUE_ENQUEUEVEBOX      = 0x1508,
    CM_FN_CMQUEUE_ENQUEUEWITHHINTS  = 0x1509,
    CM_FN_CMQUEUE_ENQUEUEFAST       = 0x150a,
    CM_FN_CMQUEUE_DESTROYEVENTFAST  = 0x150b,
    CM_FN_CMQUEUE_ENQUEUEWITHGROUPFAST = 0x150c,
};

//*-----------------------------------------------------------------------------
//| Purpose:    CMRT thin layer library supported function execution
//| Return:     CM_SUCCESS if successful
//*-----------------------------------------------------------------------------
using CMRT_UMD::CmDevice;
int32_t CmThinExecuteInternal(CmDevice *device,
                        CM_FUNCTION_ID cmFunctionID,
                        void *inputData,
                        uint32_t inputDataLen);

// Below APIs are called in CmThinExecute(), so they are declared here again.
extern int32_t CreateCmDevice(MOS_CONTEXT *mosContext,
                              CmDevice* &device,
                              uint32_t devCreateOption);

extern int32_t DestroyCmDevice(CmDevice* &device);

namespace CMRT_UMD
{
// class of CmWrapperEx for functionality extention in cm wrapper
class CmWrapperEx
{
public:
    CmWrapperEx(){}
    virtual ~CmWrapperEx(){}

    virtual void Initialize(void *context);
    virtual int Execute(
                CmDevice *device,
                CM_FUNCTION_ID cmFunctionID,
                void *inputData,
                uint32_t inputDataLen);
};
};
#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMWRAPPER_H_
