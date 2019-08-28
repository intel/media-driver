/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_device_rt_os.cpp
//! \brief     Contains Linux-dependent CmDeviceRT member functions.
//!

#include "cm_device_rt.h"

#include "cm_hal.h"
#include "cm_surface_manager.h"
#include "cm_mem.h"
#include "cm_surface_2d_rt.h"

extern int32_t CmFillMosResource(VASurfaceID, VADriverContext*, PMOS_RESOURCE);

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of CmDevice
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmDeviceRT::CmDeviceRT(uint32_t options) : CmDeviceRTBase(options)
{
    ConstructOSSpecific(options);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of CmDevice
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmDeviceRT::~CmDeviceRT()
{
    m_mosContext->SkuTable.reset();
    m_mosContext->WaTable.reset();

    DestructCommon();

    DestroyAuxDevice();
};

//*-----------------------------------------------------------------------------
//| Purpose:    Create Cm Device
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::Create(MOS_CONTEXT *umdContext,
                           CmDeviceRT* &device,
                           uint32_t options)
{
    int32_t result = CM_FAILURE;

    if (device != nullptr)
    {
        // if the Cm Device exists
        device->Acquire();
        return CM_SUCCESS;
    }

    device = new (std::nothrow) CmDeviceRT(options);
    if (device)
    {
        device->Acquire(); // increase ref count
        result = device->Initialize(umdContext);
        if (result != CM_SUCCESS)
        {
            CM_ASSERTMESSAGE("Error: Failed to initialzie CmDevice.");
            CmDeviceRT::Destroy(device);
            device = nullptr;
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmDevice due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

//*-----------------------------------------------------------------------------
//! Destroy the CmDevice_RT and kernels, samplers and the queue it created.
//! Also destroy all surfaces it created if the surface hasn't been explicitly destroyed.
//! Input :
//!     Reference to the pointer to the CmDevice_RT .
//! OUTPUT :
//!     CM_SUCCESS if CmDevice_RT is successfully destroyed.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::Destroy(CmDeviceRT* &device)
{
    INSERT_API_CALL_LOG();

    int32_t result = CM_SUCCESS;

    int32_t refCount = device->Release();

    if (refCount == 0)
    {
        CmSafeDelete(device);
    }

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize the OS-Specific part in the Initialize() function
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::InitializeOSSpecific(MOS_CONTEXT *mosContext)
{
    return CreateAuxDevice( mosContext);
}

void CmDeviceRT::ConstructOSSpecific(uint32_t devCreateOption)
{
    m_pfnReleaseVaSurface = nullptr;

    // If use dynamic states.
    m_cmHalCreateOption.dynamicStateHeap = (devCreateOption & CM_DEVICE_CONFIG_DSH_DISABLE_MASK) ? false : true;
    if (m_cmHalCreateOption.dynamicStateHeap)
    {
        m_cmHalCreateOption.maxTaskNumber = 64;
    }
    return;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Intel CM Device and Get maxValues and version of CM device
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::CreateAuxDevice(MOS_CONTEXT *mosContext)  //VADriverContextP
{
    int32_t                     hr = CM_SUCCESS;
    PDDI_MEDIA_CONTEXT          mediaContext = nullptr;
    VAContextID                 vaCtxID;
    PCM_HAL_STATE               cmHalState;
    PCM_CONTEXT                 cmContext;

    m_mosContext = mosContext;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(HalCm_Create(mosContext, &m_cmHalCreateOption, &cmHalState ));

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmHalState->pfnCmAllocate(cmHalState));

    // allocate cmContext
    cmContext = (PCM_CONTEXT)MOS_AllocAndZeroMemory(sizeof(CM_CONTEXT));
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmContext);
    cmContext->mosCtx     = *mosContext; // mos context
    cmContext->cmHalState = cmHalState;

    m_accelData =  (void *)cmContext;

    CM_CHK_CMSTATUS_GOTOFINISH_WITH_MSG(GetMaxValueFromCaps(m_halMaxValues, m_halMaxValuesEx), "Failed to get Max values.");
    CM_CHK_CMSTATUS_GOTOFINISH_WITH_MSG(GetGenPlatform(m_platform), "Failed to get GPU type.");

    //  Get version from Driver
    m_ddiVersion = CM_VERSION;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destory Intel Aux Device : CM device
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::DestroyAuxDevice()
{
    PCM_CONTEXT_DATA  cmData = (PCM_CONTEXT_DATA)m_accelData;

    // Delete VPHAL State
    if (cmData && cmData->cmHalState)
    {
        cmData->mosCtx.SkuTable.reset();
        cmData->mosCtx.WaTable.reset();
        HalCm_Destroy(cmData->cmHalState);
        // Delete CM Data itself
        MOS_FreeMemory(cmData);

    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 2D
//| Arguments :
//|               VASurfaceID :     [in]     index to MEDIASURFACE[], same as LIBVA SurfaceID
//|               umdContext           [in]     Va driver context
//|               surface          [out]    Reference to Pointer to CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateSurface2D(VASurfaceID vaSurface,
                                              VADriverContext *vaDriverCtx,
                                              CmSurface2D* & surface)
{
    INSERT_API_CALL_LOG();

    MOS_RESOURCE mosResource;
    int32_t hr = CmFillMosResource( vaSurface, vaDriverCtx, &mosResource);
    if( hr != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Failed to fill MOS resource.");
        return hr;
    }

    CmSurface2DRT *surfaceRT = nullptr;
    hr = m_surfaceMgr->CreateSurface2DFromMosResource(&mosResource, false, surfaceRT);
    surface = surfaceRT;
    return hr;
}

//*----------------------------------------------------------------------------
//| Purpose:    Get JIT Compiler function from igfxcmjit64/32.dll
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetJITCompileFnt(pJITCompile &jitCompile)
{
    if (m_fJITCompile)
    {
        jitCompile = m_fJITCompile;
    }
    else
    {
        int ret = LoadJITDll();
        if (ret != CM_SUCCESS)
        {
            return ret;
        }
        jitCompile = m_fJITCompile;
    }
    return CM_SUCCESS;
}

int32_t CmDeviceRT::GetJITCompileFntV2(pJITCompile_v2 &jitCompile_v2)
{
    if (m_fJITCompile_v2)
    {
        jitCompile_v2 = m_fJITCompile_v2;
    }
    else
    {
        int ret = LoadJITDll();
        if (ret != CM_SUCCESS)
        {
            return ret;
        }
        jitCompile_v2 = m_fJITCompile_v2;
    }
    return CM_SUCCESS;
}


//*----------------------------------------------------------------------------
//| Purpose:    Get JIT Free Block function from igfxcmjit64/32.dll
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetFreeBlockFnt(pFreeBlock &freeBlock)
{
    if (m_fFreeBlock)
    {
        freeBlock = m_fFreeBlock;
    }
    else
    {
        int ret = LoadJITDll();
        if (ret != CM_SUCCESS)
        {
            return ret;
        }
        freeBlock = m_fFreeBlock;
    }
    return CM_SUCCESS;
}

//*----------------------------------------------------------------------------
//| Purpose:    Get JIT Version function from igfxcmjit64/32.dll, It used to get
//|             version from common isa
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetJITVersionFnt(pJITVersion &jitVersion)
{
    if (m_fJITVersion)
    {
        jitVersion = m_fJITVersion;
    }
    else
    {
        int ret = LoadJITDll();
        if (ret != CM_SUCCESS)
        {
            return ret;
        }
        jitVersion = m_fJITVersion;
    }
    return CM_SUCCESS;
}

//*----------------------------------------------------------------------------
//| Purpose:    Get all JIT  functions from igfxcmjit64/32.dll.
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::LoadJITDll()
{
    int result = 0;

    if (nullptr == m_hJITDll)
    {
        m_hJITDll = dlopen( "libigc.so", RTLD_LAZY );
        if (nullptr == m_hJITDll)
        {
            CM_NORMALMESSAGE("Warning: Failed to load IGC library, will try JIT library.");
            if (sizeof(void *) == 4)  //32-bit
            {
                m_hJITDll = dlopen( "igfxcmjit32.so", RTLD_LAZY );
            }
            else  //64-bit
            {
                m_hJITDll = dlopen( "igfxcmjit64.so", RTLD_LAZY );
            }
        }
        if (nullptr == m_hJITDll)
        {
            result = CM_JITDLL_LOAD_FAILURE;
            CM_ASSERTMESSAGE("Error: Failed to load either IGC or JIT library.");
            return result;
        }
        if ((nullptr == m_fJITCompile && nullptr == m_fJITCompile_v2) || nullptr == m_fFreeBlock || nullptr == m_fJITVersion)
        {
            m_fJITCompile = (pJITCompile)MOS_GetProcAddress(m_hJITDll, JITCOMPILE_FUNCTION_STR);
            m_fJITCompile_v2 = (pJITCompile_v2)MOS_GetProcAddress(m_hJITDll, JITCOMPILEV2_FUNCTION_STR);
            m_fFreeBlock = (pFreeBlock)MOS_GetProcAddress(m_hJITDll, FREEBLOCK_FUNCTION_STR);
            m_fJITVersion = (pJITVersion)MOS_GetProcAddress(m_hJITDll, JITVERSION_FUNCTION_STR);
        }

        if ((nullptr == m_fJITCompile && nullptr == m_fJITCompile_v2) || (nullptr == m_fFreeBlock) || (nullptr == m_fJITVersion))
        {
            result = CM_JITDLL_LOAD_FAILURE;
            CM_ASSERTMESSAGE("Error: Failed to get JIT functions.");
            return result;
        }
    }

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the GPU Infomations from Internal
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RETURN_CODE CmDeviceRT::QueryGPUInfoInternal(PCM_QUERY_CAPS queryCaps)
{
    PCM_CONTEXT_DATA        cmData;
    PCM_HAL_STATE           cmHalState;
    CM_RETURN_CODE          hr = CM_SUCCESS;

    cmData = (PCM_CONTEXT_DATA)GetAccelData();
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmData);

    cmHalState = cmData->cmHalState;
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmHalState);

    switch(queryCaps->type)
    {
        case CM_QUERY_GPU:
            queryCaps->genCore = cmHalState->platform.eRenderCoreFamily;
            break;

        case CM_QUERY_GT:
            cmHalState->cmHalInterface->GetGenPlatformInfo(nullptr, &queryCaps->genGT, nullptr);
            break;

        case CM_QUERY_MIN_RENDER_FREQ:
            queryCaps->minRenderFreq = 0;
            break;

        case CM_QUERY_MAX_RENDER_FREQ:
            queryCaps->maxRenderFreq = 0;
            break;

        case CM_QUERY_STEP:
            queryCaps->genStepId = cmHalState->platform.usRevId;
            break;

        case CM_QUERY_GPU_FREQ:
            CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmHalState->pfnGetGPUCurrentFrequency(cmHalState, &queryCaps->gpuCurrentFreq));
            break;

        default:
            hr = CM_FAILURE;
            goto finish;
        }
finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the supported formats in Surface2D from Internal
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RETURN_CODE
CmDeviceRT::QuerySurface2DFormatsInternal(PCM_QUERY_CAPS queryCaps)
{
    if (queryCaps->surface2DFormats)
    {
        CM_SURFACE_FORMAT formats[ CM_MAX_SURFACE2D_FORMAT_COUNT_INTERNAL ] =
        {
            CM_SURFACE_FORMAT_X8R8G8B8,
            CM_SURFACE_FORMAT_A8R8G8B8,
            CM_SURFACE_FORMAT_A8B8G8R8,
            CM_SURFACE_FORMAT_R32F,
            CM_SURFACE_FORMAT_V8U8,
            CM_SURFACE_FORMAT_P8,
            CM_SURFACE_FORMAT_YUY2,
            CM_SURFACE_FORMAT_A8,
            CM_SURFACE_FORMAT_NV12,
            CM_SURFACE_FORMAT_P010,
            CM_SURFACE_FORMAT_P016,
            CM_SURFACE_FORMAT_Y216,
            CM_SURFACE_FORMAT_Y416,
            CM_SURFACE_FORMAT_UYVY,
            CM_SURFACE_FORMAT_V8U8,
            CM_SURFACE_FORMAT_Y8_UNORM,
            CM_SURFACE_FORMAT_YV12,
            CM_SURFACE_FORMAT_R8_UINT,
            CM_SURFACE_FORMAT_R16_UINT,
            CM_SURFACE_FORMAT_P208,
            CM_SURFACE_FORMAT_AYUV,
            CM_SURFACE_FORMAT_Y210,
            CM_SURFACE_FORMAT_Y410,
        };
        CmSafeMemCopy( queryCaps->surface2DFormats, formats, CM_MAX_SURFACE2D_FORMAT_COUNT_INTERNAL  * sizeof( GMM_RESOURCE_FORMAT ) );
    }
    else
        return CM_FAILURE;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Report all the supported formats for surface2D
//| Returns:    No
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::QuerySurface2DFormats(void *capValue,
                                          uint32_t & capValueSize)
{
    if( capValueSize >= CM_MAX_SURFACE2D_FORMAT_COUNT  * sizeof( GMM_RESOURCE_FORMAT ) )
    {
        capValueSize = CM_MAX_SURFACE2D_FORMAT_COUNT  * sizeof( GMM_RESOURCE_FORMAT ) ;
        CM_SURFACE_FORMAT formats[ CM_MAX_SURFACE2D_FORMAT_COUNT ] =
        {
            CM_SURFACE_FORMAT_X8R8G8B8,
            CM_SURFACE_FORMAT_A8R8G8B8,
            CM_SURFACE_FORMAT_A8B8G8R8,
            CM_SURFACE_FORMAT_R32F,
            CM_SURFACE_FORMAT_V8U8,
            CM_SURFACE_FORMAT_P8,
            CM_SURFACE_FORMAT_YUY2,
            CM_SURFACE_FORMAT_A8,
            CM_SURFACE_FORMAT_NV12,
            CM_SURFACE_FORMAT_P010,
            CM_SURFACE_FORMAT_P016,
            CM_SURFACE_FORMAT_Y216,
            CM_SURFACE_FORMAT_Y416,
            CM_SURFACE_FORMAT_UYVY,
            CM_SURFACE_FORMAT_IMC3,
            CM_SURFACE_FORMAT_411P,
            CM_SURFACE_FORMAT_411R,
            CM_SURFACE_FORMAT_422H,
            CM_SURFACE_FORMAT_422V,
            CM_SURFACE_FORMAT_444P,
            CM_SURFACE_FORMAT_RGBP,
            CM_SURFACE_FORMAT_BGRP,
            CM_SURFACE_FORMAT_YV12,
            CM_SURFACE_FORMAT_R8_UINT,
            CM_SURFACE_FORMAT_R16_UINT,
            CM_SURFACE_FORMAT_P208,
            CM_SURFACE_FORMAT_AYUV,
            CM_SURFACE_FORMAT_Y210,
            CM_SURFACE_FORMAT_Y410,
        };
        CmSafeMemCopy( capValue, formats, capValueSize );
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set CM Context ID in MediaContext
//| Arguments : Context ID
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::SetVaCtxID(uint32_t vaCtxID)
{
    m_vaCtxID = vaCtxID;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get CM Context ID in MediaContext
//| Arguments : Context ID
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetVaCtxID(uint32_t &vaCtxID)
{
    vaCtxID = m_vaCtxID;
    return CM_SUCCESS;
}

int32_t CmDeviceRT::RegisterCallBack(pCallBackReleaseVaSurface callBack)
{
    m_pfnReleaseVaSurface = callBack;
    return CM_SUCCESS;
}

int32_t CmDeviceRT::ReleaseVASurface(void *vaDisplay, void *vaSurfaceID)
{
    if(m_pfnReleaseVaSurface)
    {
        m_pfnReleaseVaSurface( vaDisplay, vaSurfaceID);
    }

    return CM_SUCCESS;
}

int32_t CmDeviceRT::ReadVtuneProfilingFlag()
{
    //Aggrement with Vtune: <user home dir>/.mdf_trace
    //if .mdf_trace does not exist, vtune log is off
    //if .mdf_trace exists, read string "Output=<hexmask>"
    //hexmask = 1 : enable; hexmask = 0: disable
    m_vtuneOn = false;

    char *homeStr = getenv("HOME");
    if (homeStr == nullptr)
    {
       //Even homeStr is not found, this function returns success.
       //m_vtuneOn is still false.
       return CM_SUCCESS;
    }

    char traceFile[256];
    int offset = snprintf(traceFile, 256, "%s", homeStr);
    snprintf(traceFile+offset, 256-offset, "%s", "/.mdf_trace");

    FILE *traceFd = fopen(traceFile, "r");
    int flag = 0;
    if(traceFd )
    {
      //read data from file
      int ret = fscanf(traceFd, "Output=%d", &flag);
      if(ret >=0 && flag == 1)
      {
         m_vtuneOn = true;
      }
      fclose(traceFd);
    }

    //Set flag in cm hal layer
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)this->GetAccelData();
    PCM_HAL_STATE cmHalState = cmData->cmHalState;
    cmHalState->pfnSetVtuneProfilingFlag(cmHalState, m_vtuneOn);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create shared Surface 2D (OS agnostic)
//| Arguments :
//|               mosResource      [in]     Pointer to Mos resource
//|               surface          [out]    Reference to Pointer to CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateSurface2D(PMOS_RESOURCE mosResource,
                                              CmSurface2D* & surface)
{
    INSERT_API_CALL_LOG();

    if (mosResource == nullptr)
    {
        return CM_INVALID_MOS_RESOURCE_HANDLE;
    }

    CLock locker(m_criticalSectionSurface);

    CmSurface2DRT *surfaceRT = nullptr;
    int ret = m_surfaceMgr->CreateSurface2DFromMosResource(mosResource, false, surfaceRT);
    surface = surfaceRT;
    return ret;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 2D
//| Arguments :   width             [in]     width of the  CmSurface2D
//|               height            [in]     height of the CmSurface2D
//|               format            [in]     format of the CmSurface2D
//|               surface          [in/out]    Reference to Pointer to CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateSurface2D(uint32_t width,
                                              uint32_t height,
                                              CM_SURFACE_FORMAT format,
                                              CmSurface2D* & surface)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionSurface);

    CmSurface2DRT *surfaceRT = nullptr;
    int ret = m_surfaceMgr->CreateSurface2D(width, height, 0, true, format, surfaceRT);
    surface = surfaceRT;
    return ret;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 2D
//| NOTE: Called by CM Wrapper, from CMRT Thin
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::CreateSurface2D(PMOS_RESOURCE mosResource,
                                    bool isCmCreated,
                                    CmSurface2D* & surface)
{
    INSERT_API_CALL_LOG();

    if (mosResource == nullptr)
    {
        return CM_INVALID_MOS_RESOURCE_HANDLE;
    }

    CLock locker(m_criticalSectionSurface);

    CmSurface2DRT *surfaceRT = nullptr;
    int ret = m_surfaceMgr->CreateSurface2DFromMosResource(mosResource, isCmCreated, surfaceRT);
    surface = surfaceRT;
    return ret;
}
}  // namespace
