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
    void *pCmVmeSurfHandle;                    // [in]
    CM_VME_SURFACE_STATE_PARAM *pSurfDimPara;  // [in]
    int32_t iReturnValue;                      // [out]
};

struct CM_CREATESAMPLER_PARAM
{
    CM_SAMPLER_STATE SampleState;         // [in]
    void *pCmSamplerHandle;               // [out]
    SamplerIndex *pCmSamplerIndexHandle;  // [out]
    int32_t iReturnValue;                 // [out]
};

struct CM_CREATESAMPLER_PARAM_EX
{
    CM_SAMPLER_STATE_EX SampleState;      // [in]
    void *pCmSamplerHandle;               // [out]
    SamplerIndex *pCmSamplerIndexHandle;  // [out]
    int32_t iReturnValue;                 // [out]
};

struct CM_GETCAPS_PARAM
{
    CM_DEVICE_CAP_NAME capName;  //[in]
    uint32_t capValueSize;       //[in]
    void *pCapValue;             //[in/out]
    uint32_t iReturnValue;       //[out] Return value
};

struct CM_CREATESAMPLER8x8_PARAM
{
    CM_SAMPLER_8X8_DESCR Sample8x8Desc;   // [in]
    void *pCmSampler8x8Handle;            // [out]
    SamplerIndex *pCmSamplerIndexHandle;  // [out]
    int32_t iReturnValue;                 // [out]
};

struct CM_DESTROYSAMPLER8x8_PARAM
{
    void *pCmSampler8x8Handle;  // [in]
    int32_t iReturnValue;       // [out]
};

struct CM_CREATESAMPLER8x8SURF_PARAM
{
    void *pCmSurf2DHandle;                           // [in]
    CM_SAMPLER8x8_SURFACE CmSampler8x8Type;          // [in]
    CM_SURFACE_ADDRESS_CONTROL_MODE Sampler8x8Mode;  // [in]
    SurfaceIndex *pCmSurfIndexHandle;                // [out]
    int32_t iReturnValue;                            // [out]
};

struct CM_CREATESAMPLER8x8SURFEX_PARAM
{
    void *pCmSurf2DHandle;                           // [in]
    CM_SAMPLER8x8_SURFACE CmSampler8x8Type;          // [in]
    CM_SURFACE_ADDRESS_CONTROL_MODE Sampler8x8Mode;  // [in]
    CM_FLAG* pFlag;                                  // [in]
    SurfaceIndex* pCmSurfIndexHandle;                // [out]
    int32_t iReturnValue;                            // [out]
};

struct CM_CREATESAMPLER2DEX_PARAM
{
    void *pCmSurface2DHandle;       // [in]
    CM_FLAG *pFlag;                 // [in]
    void *pSamplerSurfIndexHandle;  // [out]
    int32_t iReturnValue;           // [out]
};

struct CM_DESTROYSAMPLER8x8SURF_PARAM
{
    SurfaceIndex *pCmSurfIndexHandle;  // [in]
    int32_t iReturnValue;              // [out]
};

struct CM_DEVICE_SETSUGGESTEDL3_PARAM
{
    L3_SUGGEST_CONFIG l3_s_c;  // [in] Cap Type
    int32_t iReturnValue;      // [out] the return value from CMRT@UMD
};

struct CM_DEVICE_SETCAP_PARAM
{
    CM_DEVICE_CAP_NAME capName;  // [in] Cap Type
    size_t capValueSize;         // [in] Value Size
    void *pCapValue;             // [in] Pointer to value
    int32_t iReturnValue;        // [out] the return value from CMRT@UMD
};

struct CM_DEVICE_GET_VISA_VERSION_PARAM
{
  uint32_t iMajorVersion;  // [OUT] the major version of jitter
  uint32_t iMinorVersion;  // [OUT] the minor version of jitter
  int32_t  iReturnValue;   // [OUT] return value
};

enum CM_BUFFER_TYPE
{
    CM_BUFFER_N   = 0,
    CM_BUFFER_UP  = 1,
    CM_BUFFER_SVM = 2
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

struct CM_DESTROYCMDEVICE_PARAM
{
    void *pCmDeviceHandle;  // [in/out] pointer to CmDevice object
    int32_t iReturnValue;   // [out] the return value from CMRT@UMD
};

struct CM_LOADPROGRAM_PARAM
{
    void *pCISACode;          // [in] pointer to the CISA code buffer
    uint32_t uiCISACodeSize;  // [in] size of CISA code
    char *options;            // [in] additonal options for LoadProgram
    void *pCmProgramHandle;   // [out] pointer to CmProgram object used by CMRT@UMD
    uint32_t indexInArray;    // [out] index in m_ProgramArray of CMRT@UMD
    int32_t iReturnValue;     // [out] the return value from CMRT@UMD
};

struct CM_DESTROYPROGRAM_PARAM
{
    void *pCmProgramHandle;  // [IN] pointer to CmProgram object used by CMRT@UMD
    int32_t iReturnValue;    // [out] the return value from CMRT@UMD
};

struct CM_CREATEKERNEL_PARAM
{
    void *pCmProgramHandle;     // [in] pointer to CmProgram used in driver
    char *pKernelName;          // [in] pointer to the kernel name string
    char *pOptions;             // [in] pointer to the kernel creation options
    void *pCmKernelHandle;      // [out] pointer to new created CmKernel used in driver
    uint32_t indexKernelArray;  // [out] index in m_KernelArray of CMRT@UMD
    int32_t iReturnValue;       // [out] the return value from driver
};

struct CM_DESTROYKERNEL_PARAM
{
    void *pCmKernelHandle;  // [in/out] pointer to new created CmKernel used in driver
    int32_t iReturnValue;   // [out] the return value from driver
};

struct CM_CREATETASK_PARAM
{
    void *pCmTaskHandle;   // [out] pointer to new created CmTask used in driver
    uint32_t iTaskIndex;   // [out] index of task in task array
    int32_t iReturnValue;  // [out] the return value from driver
};

struct CM_DESTROYTASK_PARAM
{
    void *pCmTaskHandle;   // [in/out] pointer to CmTask used in driver
    int32_t iReturnValue;  // [out] the return value from driver
};

struct CM_CREATETHREADSPACE_PARAM
{
    uint32_t TsWidth;         // [in]
    uint32_t TsHeight;        // [in]
    void *pCmTsHandle;        // [out]
    uint32_t indexInTSArray;  // [out]
    int32_t iReturnValue;     // [out]
};

struct CM_DESTROYTHREADSPACE_PARAM
{
    void *pCmTsHandle;     // [in]
    int32_t iReturnValue;  // [out]
};

struct CM_DESTROYVMESURFACE_PARAM
{
    void *pCmVmeSurfIndexHandle;  // [in]
    int32_t iReturnValue;         // [out]
};

struct CM_CREATEVMESURFACE_PARAM
{
    void *pCmCurSurfHandle;       // [in]
    void *pCmForwardSurfArray;    // [in]
    void *pCmBackwardSurfArray;   // [in]
    uint32_t iForwardSurfCount;   // [in]
    uint32_t iBackwardSurfCount;  // [in]
    void *pCmVmeSurfIndexHandle;  // [out]
    int32_t iReturnValue;         // [out]
};

struct CM_DESTROYSAMPLER_PARAM
{
    void *pCmSamplerHandle;  // [in]
    int32_t iReturnValue;    // [out]
};

struct CM_CREATETGROUPSPACE_PARAM
{
    uint32_t thrdSpaceWidth;   // [in]
    uint32_t thrdSpaceHeight;  // [in]
    uint32_t thrdSpaceDepth;   // [in]
    uint32_t grpSpaceWidth;    // [in]
    uint32_t grpSpaceHeight;   // [in]
    uint32_t grpSpaceDepth;    // [in]
    void *pCmGrpSpaceHandle;   // [out]
    uint32_t iTGSIndex;        // [out]
    int32_t iReturnValue;      // [out]
};

struct CM_DESTROYTGROPUSPACE_PARAM
{
    void *pCmGrpSpaceHandle;  // [in]
    int32_t iReturnValue;     // [out]
};

struct CM_GETSURFACE2DINFO_PARAM
{
    uint32_t iWidth;           // [in] Surface Width
    uint32_t iHeight;          // [in] Surface Height
    CM_SURFACE_FORMAT format;  // [in] Surface Format
    uint32_t iPitch;           // [out] Pitch
    uint32_t iPhysicalSize;    // [out] Physical size
    uint32_t iReturnValue;     // [out] Return value
};

struct CM_CREATESAMPLER2D_PARAM
{
    void *pCmSurface2DHandle;       // [in] pointer to CmSurface2D object used by CMRT@UMD
    void *pSamplerSurfIndexHandle;  // [out] pointer of SurfaceIndex used in driver
    int32_t iReturnValue;           // [out] the return value from CMRT@UMD
};

struct CM_CREATESAMPLER2DUP_PARAM
{
    void *pCmSurface2DHandle;       // [in] pointer to CmSurface2DUP object used by CMRT@UMD
    void *pSamplerSurfIndexHandle;  // [out] pointer of SurfaceIndex used in driver
    int32_t iReturnValue;            // [out] the return value from CMRT@UMD
};

struct CM_CREATESAMPLER3D_PARAM
{
    void *pCmSurface3DHandle;       // [in] pointer to CmSurface3D object used by CMRT@UMD
    void *pSamplerSurfIndexHandle;  // [out] pointer of SurfaceIndex used in driver
    int32_t iReturnValue;           // [out] the return value from CMRT@UMD
};

struct CM_DESTROYSAMPLERSURF_PARAM
{
    void *pSamplerSurfIndexHandle;  // [in] pointer of SamplerSurfaceIndex used in driver
    int32_t iReturnValue;           // [out] the return value from CMRT@UMD
};

struct CM_DEVICE_INIT_PRINT_BUFFER_PARAM
{
    uint32_t dwPrintBufferSize;  // [in]  print buffer's size
    void *pPrintBufferMem;       // [out] print buffer's memory
    int32_t iReturnValue;        //[out] return value
};

struct CM_CREATEVEBOX_PARAM
{
    void *pCmVeboxHandle;        // [out] CmVeboxG75's handle
    uint32_t indexInVeboxArray;  // [out] index in m_VeboxArray
    int32_t iReturnValue;        // [out] return value
};

struct CM_DESTROYVEBOX_PARAM
{
    void *pCmVeboxHandle;  // [IN] CmVeboxG75's handle
    int32_t iReturnValue;  // [out] return value
};

struct CM_CLONE_KERNEL_PARAM
{
    void *pCmKernelHandleSrc;   // [IN] source kernel
    void *pCmKernelHandleDest;  // [IN] dest kernel
    int32_t iReturnValue;       // [out] return value
};

struct CM_DEVICE_CREATE_SURF2D_ALIAS_PARAM
{
    void *pCmSurface2DHandle;   // [IN] pointer to CmSurface2D object used by CMRT@UMD
    void *pSurfaceIndexHandle;  // [OUT] ponter of SurfaceIndex
    int32_t iReturnValue;       // [OUT] return value
};

struct CM_DEVICE_CREATE_BUFFER_ALIAS_PARAM
{
    void *pCmBufferHandle;      // [IN] pointer to CmBuffer object used by CMRT@UMD
    void *pSurfaceIndexHandle;  // [OUT] ponter of SurfaceIndex
    int32_t iReturnValue;       // [OUT] return value
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
