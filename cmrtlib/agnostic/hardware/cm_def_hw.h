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
#ifndef CMRTLIB_AGNOSTIC_HARDWARE_CM_DEF_HW_H_
#define CMRTLIB_AGNOSTIC_HARDWARE_CM_DEF_HW_H_

#include "cm_def.h"
#include "cm_device_base.h"
#include "cm_l3_cache_config.h"

class SamplerIndex;

struct CM_CONFIGVMESURFACEDIMENSION_PARAM
{
    void *cmVmeSurfHandle;                    // [in]
    CM_VME_SURFACE_STATE_PARAM *surfDimPara;  // [in]
    int32_t returnValue;                      // [out]
};

struct CM_CREATESAMPLER_PARAM
{
    CM_SAMPLER_STATE samplerState;        // [in]
    void *cmSamplerHandle;               // [out]
    SamplerIndex *cmSamplerIndexHandle;  // [out]
    int32_t returnValue;                 // [out]
};

struct CM_CREATESAMPLER_PARAM_EX
{
    CM_SAMPLER_STATE_EX samplerState;     // [in]
    void *cmSamplerHandle;               // [out]
    SamplerIndex *cmSamplerIndexHandle;  // [out]
    int32_t returnValue;                 // [out]
};

struct CM_GETCAPS_PARAM
{
    CM_DEVICE_CAP_NAME capName;  //[in]
    uint32_t capValueSize;       //[in]
    void *capValue;              //[in/out]
    uint32_t returnValue;        //[out] Return value
};

struct CM_CREATESAMPLER8x8_PARAM
{
    CM_SAMPLER_8X8_DESCR sampler8x8Desc;  // [in]
    void *cmSampler8x8Handle;            // [out]
    SamplerIndex *cmSamplerIndexHandle;  // [out]
    int32_t returnValue;                 // [out]
};

struct CM_DESTROYSAMPLER8x8_PARAM
{
    void *cmSampler8x8Handle;  // [in]
    int32_t returnValue;       // [out]
};

struct CM_CREATESAMPLER8x8SURF_PARAM
{
    void *cmSurf2DHandle;                           // [in]
    CM_SAMPLER8x8_SURFACE cmSampler8x8Type;         // [in]
    CM_SURFACE_ADDRESS_CONTROL_MODE sampler8x8Mode; // [in]
    SurfaceIndex *cmSurfIndexHandle;                // [out]
    int32_t returnValue;                            // [out]
};

struct CM_CREATESAMPLER8x8SURFEX_PARAM
{
    void *cmSurf2DHandle;                           // [in]
    CM_SAMPLER8x8_SURFACE cmSampler8x8Type;         // [in]
    CM_SURFACE_ADDRESS_CONTROL_MODE sampler8x8Mode; // [in]
    CM_FLAG* flag;                                  // [in]
    SurfaceIndex* cmSurfIndexHandle;                // [out]
    int32_t returnValue;                            // [out]
};

struct CM_CREATESAMPLER2DEX_PARAM
{
    void *cmSurface2DHandle;       // [in]
    CM_FLAG *flag;                 // [in]
    void *samplerSurfIndexHandle;  // [out]
    int32_t returnValue;           // [out]
};

struct CM_DESTROYSAMPLER8x8SURF_PARAM
{
    SurfaceIndex *cmSurfIndexHandle;  // [in]
    int32_t returnValue;              // [out]
};

struct CM_DEVICE_SETSUGGESTEDL3_PARAM
{
    L3_SUGGEST_CONFIG l3SuggestConfig;  // [in] Cap Type
    int32_t returnValue;      // [out] the return value from CMRT@UMD
};

struct CM_DEVICE_SETCAP_PARAM
{
    CM_DEVICE_CAP_NAME capName;  // [in] Cap Type
    size_t  capValueSize;         // [in] Value Size
    void    *capValue;            // [in] Pointer to value
    int32_t returnValue;         // [out] the return value from CMRT@UMD
};

struct CM_DEVICE_GET_VISA_VERSION_PARAM
{
  uint32_t majorVersion;  // [OUT] the major version of jitter
  uint32_t minorVersion;  // [OUT] the minor version of jitter
  int32_t  returnValue;   // [OUT] return value
};

enum CM_BUFFER_TYPE
{
    CM_BUFFER_N   = 0,
    CM_BUFFER_UP  = 1,
    CM_BUFFER_SVM = 2,
    CM_BUFFER_STATELESS = 5
};

struct CM_CREATEBUFFER_PARAM
{
    size_t size;              // [in]  buffer size in byte
    CM_BUFFER_TYPE bufferType;  // [in]  Buffer type (Buffer, BufferUP, Buffer SVM, Buffer Stateless)
    void *sysMem;               // [in]  Address of system memory
    void *cmBufferHandle;       // [out] pointer to CmBuffer object in CMRT@UMD
    int32_t returnValue;        // [out] the return value from CMRT@UMD
    uint32_t option;
};

struct CM_DESTROYCMDEVICE_PARAM
{
    void *cmDeviceHandle;  // [in/out] pointer to CmDevice object
    int32_t returnValue;   // [out] the return value from CMRT@UMD
};

struct CM_LOADPROGRAM_PARAM
{
    void *cisaCode;          // [in] pointer to the CISA code buffer
    uint32_t cisaCodeSize;   // [in] size of CISA code
    char *options;           // [in] additonal options for LoadProgram
    void *cmProgramHandle;   // [out] pointer to CmProgram object used by CMRT@UMD
    uint32_t indexInArray;   // [out] index in m_ProgramArray of CMRT@UMD
    int32_t returnValue;     // [out] the return value from CMRT@UMD
};

struct CM_DESTROYPROGRAM_PARAM
{
    void *cmProgramHandle;  // [IN] pointer to CmProgram object used by CMRT@UMD
    int32_t returnValue;    // [out] the return value from CMRT@UMD
};

struct CM_CREATEKERNEL_PARAM
{
    void *cmProgramHandle;     // [in] pointer to CmProgram used in driver
    char *kernelName;          // [in] pointer to the kernel name string
    char *options;             // [in] pointer to the kernel creation options
    void *cmKernelHandle;      // [out] pointer to new created CmKernel used in driver
    uint32_t indexKernelArray; // [out] index in m_KernelArray of CMRT@UMD
    int32_t  returnValue;      // [out] the return value from driver
};

struct CM_DESTROYKERNEL_PARAM
{
    void *cmKernelHandle;  // [in/out] pointer to new created CmKernel used in driver
    int32_t returnValue;   // [out] the return value from driver
};

struct CM_CREATETASK_PARAM
{
    void *cmTaskHandle;   // [out] pointer to new created CmTask used in driver
    uint32_t taskIndex;   // [out] index of task in task array
    int32_t returnValue;  // [out] the return value from driver
};

struct CM_DESTROYTASK_PARAM
{
    void *cmTaskHandle;   // [in/out] pointer to CmTask used in driver
    int32_t returnValue;  // [out] the return value from driver
};

struct CM_CREATETHREADSPACE_PARAM
{
    uint32_t tsWidth;         // [in]
    uint32_t tsHeight;        // [in]
    void *cmTsHandle;         // [out]
    uint32_t indexInTSArray;  // [out]
    int32_t returnValue;      // [out]
};

struct CM_DESTROYTHREADSPACE_PARAM
{
    void *cmTsHandle;     // [in]
    int32_t returnValue;  // [out]
};

struct CM_DESTROYVMESURFACE_PARAM
{
    void *cmVmeSurfIndexHandle;  // [in]
    int32_t returnValue;         // [out]
};

struct CM_CREATEVMESURFACE_PARAM
{
    void *cmCurSurfHandle;       // [in]
    void *cmForwardSurfArray;    // [in]
    void *cmBackwardSurfArray;   // [in]
    uint32_t forwardSurfCount;   // [in]
    uint32_t backwardSurfCount;  // [in]
    void *cmVmeSurfIndexHandle;  // [out]
    int32_t returnValue;         // [out]
};

struct CM_DESTROYSAMPLER_PARAM
{
    void *cmSamplerHandle;  // [in]
    int32_t returnValue;    // [out]
};

struct CM_CREATETGROUPSPACE_PARAM
{
    uint32_t thrdSpaceWidth;   // [in]
    uint32_t thrdSpaceHeight;  // [in]
    uint32_t thrdSpaceDepth;   // [in]
    uint32_t grpSpaceWidth;    // [in]
    uint32_t grpSpaceHeight;   // [in]
    uint32_t grpSpaceDepth;    // [in]
    void     *cmGrpSpaceHandle;    // [out]
    uint32_t tgsIndex;         // [out]
    int32_t  returnValue;       // [out]
};

struct CM_DESTROYTGROPUSPACE_PARAM
{
    void *cmGrpSpaceHandle;  // [in]
    int32_t returnValue;     // [out]
};

struct CM_GETSURFACE2DINFO_PARAM
{
    uint32_t width;           // [in] Surface Width
    uint32_t height;          // [in] Surface Height
    CM_SURFACE_FORMAT format; // [in] Surface Format
    uint32_t pitch;           // [out] Pitch
    uint32_t physicalSize;    // [out] Physical size
    uint32_t returnValue;     // [out] Return value
};

struct CM_CREATESAMPLER2D_PARAM
{
    void *cmSurface2DHandle;       // [in] pointer to CmSurface2D object used by CMRT@UMD
    void *samplerSurfIndexHandle;  // [out] pointer of SurfaceIndex used in driver
    int32_t returnValue;           // [out] the return value from CMRT@UMD
};

struct CM_CREATESAMPLER2DUP_PARAM
{
    void *cmSurface2DHandle;       // [in] pointer to CmSurface2DUP object used by CMRT@UMD
    void *samplerSurfIndexHandle;  // [out] pointer of SurfaceIndex used in driver
    int32_t returnValue;           // [out] the return value from CMRT@UMD
};

struct CM_CREATESAMPLER3D_PARAM
{
    void *cmSurface3DHandle;       // [in] pointer to CmSurface3D object used by CMRT@UMD
    void *samplerSurfIndexHandle;  // [out] pointer of SurfaceIndex used in driver
    int32_t returnValue;           // [out] the return value from CMRT@UMD
};

struct CM_DESTROYSAMPLERSURF_PARAM
{
    void *samplerSurfIndexHandle;  // [in] pointer of SamplerSurfaceIndex used in driver
    int32_t returnValue;           // [out] the return value from CMRT@UMD
};

struct CM_DEVICE_INIT_PRINT_BUFFER_PARAM
{
    uint32_t printBufferSize;   // [in] print buffer's size
    void *printBufferMem;       // [out] print buffer's memory
    int32_t returnValue;        //[out] return value
};

struct CM_DEVICE_FLUSH_PRINT_BUFFER_PARAM
{
    const char          *fileName;                //[in] target file name
    int32_t             returnValue;              //[out] return value
};

struct CM_CREATEVEBOX_PARAM
{
    void *cmVeboxHandle;         // [out] CmVeboxG75's handle
    uint32_t indexInVeboxArray;  // [out] index in m_VeboxArray
    int32_t returnValue;         // [out] return value
};

struct CM_DESTROYVEBOX_PARAM
{
    void *cmVeboxHandle;  // [IN] CmVeboxG75's handle
    int32_t returnValue;  // [out] return value
};

struct CM_CLONE_KERNEL_PARAM
{
    void *cmKernelHandleSrc;   // [IN] source kernel
    void *cmKernelHandleDest;  // [IN] dest kernel
    int32_t returnValue;       // [out] return value
};

struct CM_DEVICE_CREATE_SURF2D_ALIAS_PARAM
{
    void *cmSurface2DHandle;   // [IN] pointer to CmSurface2D object used by CMRT@UMD
    void *surfaceIndexHandle;  // [OUT] ponter of SurfaceIndex
    int32_t returnValue;       // [OUT] return value
};

struct CM_DEVICE_CREATE_BUFFER_ALIAS_PARAM
{
    void *cmBufferHandle;      // [IN] pointer to CmBuffer object used by CMRT@UMD
    void *surfaceIndexHandle;  // [OUT] ponter of SurfaceIndex
    int32_t returnValue;       // [OUT] return value
};

#ifdef _DEBUG
#define MDF_PROFILER_ENABLED 1
#endif

#if MDF_PROFILER_ENABLED
#define INSERT_PROFILER_RECORD()     CmTimer Time(__FUNCTION__ )
#else
#define INSERT_PROFILER_RECORD()
#endif

#endif  // #ifndef CMRTLIB_AGNOSTIC_HARDWARE_CM_DEF_HW_H_
