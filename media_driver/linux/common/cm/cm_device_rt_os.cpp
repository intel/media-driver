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
//| Purpose:    Destructor of CmDevice
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmDeviceRT::~CmDeviceRT()
{
    DestructCommon();

    DestroyAuxDevice();
};

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize the OS-Specific part in the Initialize() function
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::InitializeOSSpecific(MOS_CONTEXT *pUmdContext)
{
    return CreateAuxDevice( pUmdContext);
}

void CmDeviceRT::ConstructOSSpecific(uint32_t devCreateOption)
{
    m_pfnReleaseVaSurface = nullptr;

    // If use dynamic states.
    m_DevCreateOption.dynamicStateHeap = (devCreateOption & CM_DEVICE_CONFIG_DSH_DISABLE_MASK) ? false : true;
    if (m_DevCreateOption.dynamicStateHeap)
    {
        m_DevCreateOption.maxTaskNumber = 64;
    }
    return;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Intel CM Device and Get MaxValues and version of CM device
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::CreateAuxDevice(MOS_CONTEXT *pUmdContext)  //VADriverContextP
{
    int32_t                     hr = CM_SUCCESS;
    PDDI_MEDIA_CONTEXT          pMediaCtx = nullptr;
    VAContextID                 vaCtxID;
    PCM_HAL_STATE               pCmHalState;
    PCM_CONTEXT                 pCmCtx;

    m_pUmdContext = pUmdContext;

    CHK_MOSSTATUS_RETURN_CMERROR(HalCm_Create(pUmdContext, &m_DevCreateOption, &pCmHalState ));

    CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pfnCmAllocate(pCmHalState));

    // allocate pCmCtx
    pCmCtx = (PCM_CONTEXT)MOS_AllocAndZeroMemory(sizeof(CM_CONTEXT));
    CMCHK_NULL(pCmCtx);
    pCmCtx->mosCtx     = *pUmdContext; // mos context
    pCmCtx->cmHalState = pCmHalState;

    m_pAccelData =  (void *)pCmCtx;

    CMCHK_HR_MESSAGE(GetMaxValueFromCaps(m_HalMaxValues, m_HalMaxValuesEx), "Failed to get Max values.");
    CMCHK_HR_MESSAGE(GetGenPlatform(m_Platform), "Failed to get GPU type.");

    //  Get version from Driver
    m_DDIVersion = CM_VERSION;

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Destory Intel Aux Device : CM device
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::DestroyAuxDevice()
{
    PCM_CONTEXT_DATA  pCmData = (PCM_CONTEXT_DATA)m_pAccelData;

    // Delete VPHAL State
    if (pCmData && pCmData->cmHalState)
    {
        pCmData->mosCtx.SkuTable.reset();
        pCmData->mosCtx.WaTable.reset();
        HalCm_Destroy(pCmData->cmHalState);
        // Delete CM Data itself
        MOS_FreeMemory(pCmData);

    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 2D
//| Arguments :
//|               VASurfaceID :     [in]     index to MEDIASURFACE[], same as LIBVA SurfaceID
//|               pUMDCtx           [in]     Va driver context
//|               pSurface          [out]    Reference to Pointer to CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateSurface2D(VASurfaceID iVASurface,
                                              VADriverContext *pUMDCtx,
                                              CmSurface2D* & pSurface)
{
    INSERT_API_CALL_LOG();

    MOS_RESOURCE MosResource;
    int32_t hr = CmFillMosResource( iVASurface, pUMDCtx, &MosResource);
    if( hr != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Failed to fill MOS resource.");
        return hr;
    }

    CmSurface2DRT *pSurfaceRT = nullptr;
    hr = m_pSurfaceMgr->CreateSurface2D(&MosResource, false, pSurfaceRT);
    pSurface = pSurfaceRT;
    return hr;
}

CM_RT_API int32_t CmDeviceRT::DestroySurface(CmSurface2D* & pSurface)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Surface);

    int32_t status = CM_SUCCESS;

    CmSurface2DRT *pSurfaceRT = static_cast<CmSurface2DRT *>(pSurface);
    status = m_pSurfaceMgr->DestroySurface( pSurfaceRT, APP_DESTROY);

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE may be returned, which should be treated as SUCCESS.
    {
        pSurface = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }

    return status;
}


//*----------------------------------------------------------------------------
//| Purpose:    Get JIT Compiler function from igfxcmjit64/32.dll
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetJITCompileFnt(pJITCompile &fJITCompile)
{
    if (m_fJITCompile)
    {
        fJITCompile = m_fJITCompile;
    }
    else
    {
        int ret = LoadJITDll();
        if (ret != CM_SUCCESS)
        {
            return ret;
        }
        fJITCompile = m_fJITCompile;
    }
    return CM_SUCCESS;
}

//*----------------------------------------------------------------------------
//| Purpose:    Get JIT Free Block function from igfxcmjit64/32.dll
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetFreeBlockFnt(pFreeBlock &fFreeBlock)
{
    if (m_fFreeBlock)
    {
        fFreeBlock = m_fFreeBlock;
    }
    else
    {
        int ret = LoadJITDll();
        if (ret != CM_SUCCESS)
        {
            return ret;
        }
        fFreeBlock = m_fFreeBlock;
    }
    return CM_SUCCESS;
}

//*----------------------------------------------------------------------------
//| Purpose:    Get JIT Version function from igfxcmjit64/32.dll, It used to get
//|             version from common isa
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetJITVersionFnt(pJITVersion &fJITVersion)
{
    if (m_fJITVersion)
    {
        fJITVersion = m_fJITVersion;
    }
    else
    {
        int ret = LoadJITDll();
        if (ret != CM_SUCCESS)
        {
            return ret;
        }
        fJITVersion = m_fJITVersion;
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
        if (nullptr == m_fJITCompile || nullptr == m_fFreeBlock || nullptr == m_fJITVersion)
        {
            m_fJITCompile = (pJITCompile)MOS_GetProcAddress(m_hJITDll, JITCOMPILE_FUNCTION_STR);
            m_fFreeBlock = (pFreeBlock)MOS_GetProcAddress(m_hJITDll, FREEBLOCK_FUNCTION_STR);
            m_fJITVersion = (pJITVersion)MOS_GetProcAddress(m_hJITDll, JITVERSION_FUNCTION_STR);
        }

        if ((NULL==m_fJITCompile) || (NULL==m_fFreeBlock) || (NULL==m_fJITVersion))
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
CM_RETURN_CODE CmDeviceRT::QueryGPUInfoInternal(PCM_QUERY_CAPS pQueryCaps)
{
    PCM_CONTEXT_DATA        pCmData;
    PCM_HAL_STATE           pCmHalState;
    CM_RETURN_CODE          hr = CM_SUCCESS;

    pCmData = (PCM_CONTEXT_DATA)GetAccelData();
    CMCHK_NULL(pCmData);

    pCmHalState = pCmData->cmHalState;
    CMCHK_NULL(pCmHalState);

    switch(pQueryCaps->type)
    {
        case CM_QUERY_GPU:
            pQueryCaps->genCore = pCmHalState->Platform.eRenderCoreFamily;
            break;

        case CM_QUERY_GT:
            pCmHalState->pCmHalInterface->GetGenPlatformInfo(nullptr, &pQueryCaps->genGT, nullptr);
            break;

        case CM_QUERY_MIN_RENDER_FREQ:
            pQueryCaps->minRenderFreq = 0;
            break;

        case CM_QUERY_MAX_RENDER_FREQ:
            pQueryCaps->maxRenderFreq = 0;
            break;

        case CM_QUERY_STEP:
            pQueryCaps->genStepId = pCmHalState->Platform.usRevId;
            break;

        case CM_QUERY_GPU_FREQ:
            CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pfnGetGPUCurrentFrequency(pCmHalState, &pQueryCaps->gpuCurrentFreq));
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
CmDeviceRT::QuerySurface2DFormatsInternal(PCM_QUERY_CAPS pQueryCaps)
{
    if (pQueryCaps->surface2DFormats)
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
            CM_SURFACE_FORMAT_UYVY,
            CM_SURFACE_FORMAT_V8U8,
            CM_SURFACE_FORMAT_Y8_UNORM,
            CM_SURFACE_FORMAT_YV12,
            CM_SURFACE_FORMAT_R8_UINT,
            CM_SURFACE_FORMAT_R16_UINT,
        };
        CmSafeMemCopy( pQueryCaps->surface2DFormats, formats, CM_MAX_SURFACE2D_FORMAT_COUNT_INTERNAL  * sizeof( GMM_RESOURCE_FORMAT ) );
    }
    else
        return CM_FAILURE;

    return CM_SUCCESS;
}


//*-----------------------------------------------------------------------------
//| Purpose:   Report all the supported formats for surface2D
//| Returns:    No
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::QuerySurface2DFormats(void *pCapValue,
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
            CM_SURFACE_FORMAT_UYVY,
            CM_SURFACE_FORMAT_IMC3,
            CM_SURFACE_FORMAT_411P,
            CM_SURFACE_FORMAT_422H,
            CM_SURFACE_FORMAT_422V,
            CM_SURFACE_FORMAT_444P,
            CM_SURFACE_FORMAT_YV12,
            CM_SURFACE_FORMAT_R8_UINT,
            CM_SURFACE_FORMAT_R16_UINT,
        };
        CmSafeMemCopy( pCapValue, formats, capValueSize );
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
    m_VaCtxID = vaCtxID;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get CM Context ID in MediaContext
//| Arguments : Context ID
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetVaCtxID(uint32_t &vaCtxID)
{
    vaCtxID = m_VaCtxID;
    return CM_SUCCESS;
}


int32_t CmDeviceRT::RegisterCallBack(pCallBackReleaseVaSurface pCallBack)
{
    m_pfnReleaseVaSurface = pCallBack;
    return CM_SUCCESS;
}

int32_t CmDeviceRT::ReleaseVASurface(void *pVaDpy, void *pVaSurID)
{
    if(m_pfnReleaseVaSurface)
    {
        m_pfnReleaseVaSurface( pVaDpy, pVaSurID);
    }

    return CM_SUCCESS;
}

int32_t CmDeviceRT::ReadVtuneProfilingFlag()
{
    //Aggrement with Vtune: <user home dir>/.mdf_trace
    //if .mdf_trace does not exist, vtune log is off
    //if .mdf_trace exists, read string "Output=<hexmask>"
    //hexmask = 1 : enable; hexmask = 0: disable
    m_bVtuneOn = false;

    char *home_str = getenv("HOME");
    if (home_str == nullptr)
    {
       //Even home_str is not found, this function returns success.
       //m_bVtuneOn is still false.
       return CM_SUCCESS;
    }

    char trace_file[256];
    int offset = snprintf(trace_file, 256, "%s", home_str);
    snprintf(trace_file+offset, 256-offset, "%s", "/.mdf_trace");

    FILE *trace_fd = fopen(trace_file, "r");
    uint flag = 0;
    if(trace_fd )
    {
      //read data from file
      int ret = fscanf(trace_fd, "Output=%d", &flag);
      if(ret >=0 && flag == 1)
      {
         m_bVtuneOn = true;
      }
      fclose(trace_fd);
    }

    //Set flag in cm hal layer
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)this->GetAccelData();
    PCM_HAL_STATE pCmHalState = pCmData->cmHalState;
    pCmHalState->pfnSetVtuneProfilingFlag(pCmHalState, m_bVtuneOn);

    return CM_SUCCESS;
}
}  // namespace
