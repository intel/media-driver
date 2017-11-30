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

/**-----------------------------------------------------------------------------
***
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
*** ----------------------------------------------------------------------------
*/
#include "cm_surface_manager.h"
#include "cm_debug.h"
#include "cm_mem.h"
#include "cm_device.h"

enum DESTROY_KIND
{
    APP_DESTROY         = 0,
    GC_DESTROY          = 1,
    FORCE_DESTROY       = 2,
    DELAYED_DESTROY     = 3,
    THIN_DESTROY        = 4
};

struct CM_DESTROYSURFACE2D_PARAM
{
    void *pCmSurface2DHandle;  // [in/out] pointer to CmSurface2D object
    int32_t iReturnValue;      // [out] the return value from CMRT@UMD
};

struct CM_DESTROYSURFACE2DUP_PARAM
{
    void *pCmSurface2DUPHandle;  // [in/out] pointer to CmSurface2D object
    int32_t iReturnValue;        // [out] the return value from CMRT@UMD
};

struct CM_DESTROY_SURFACE3D_PARAM
{
    void *pCmSurface3DHandle;  // [in] pointer of CmSurface3D used in driver
    int32_t iReturnValue;      // [out] the return value from driver
};

struct CM_DESTROYBUFFER_PARAM
{
    void *pCmBufferHandle;  // [in/out] pointer to CmBuffer object
    int32_t iReturnValue;   // [out] the return value from CMRT@UMD
};

struct CM_CREATESURFACE2DUP_PARAM
{
    uint32_t iWidth;             // [in] width of 2D texture in pixel
    uint32_t iHeight;            // [in] height of 2D texture in pixel
    CM_SURFACE_FORMAT Format;    // [in] DXGI format of 2D texture
    void *pSysMem;               // [in] Pointer to system memory
    void *pCmSurface2DUPHandle;  // [out] pointer of CmSurface2D used in driver
    int32_t iReturnValue;        // [out] the return value from driver
};

struct CM_CREATE_SURFACE3D_PARAM
{
    uint32_t iWidth;            // [in] width of 3D  in pixel
    uint32_t iHeight;           // [in] height of 3D  in pixel
    uint32_t iDepth;            // [in] depth of 3D surface in pixel
    CM_SURFACE_FORMAT Format;   // [in] DXGI format of 3D texture
    void *pCmSurface3DHandle;   // [out] pointer of CmSurface3D used in driver
    int32_t iReturnValue;       // [out] the return value from driver
};

CmSurfaceManager::CmSurfaceManager(CmDevice_RT *device)
{
    m_device = device;
}

CmSurfaceManager::~CmSurfaceManager() {}

int32_t CmSurfaceManager::DestroySurface(CmSurface2D *&surface)
{
    int32_t result = CM_SUCCESS;
    CHK_NULL(surface);

    //Destroy surface in UMD
    CHK_RET(DestroySurface2DInUmd(surface));
    surface = nullptr;

finish:
    return result;
}

int32_t CmSurfaceManager::DestroySurface2DInUmd(CmSurface2D *&surface)
{
    int32_t result = CM_SUCCESS;

    CHK_NULL_RETURN(surface);
    //Call Into CMRT@UMD to free Surface
    CM_DESTROYSURFACE2D_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_DESTROYSURFACE2D_PARAM));
    inParam.pCmSurface2DHandle = surface;

    result = m_device->OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYSURFACE2D,
                                            &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(result);
    return inParam.iReturnValue;
}

int32_t CmSurfaceManager::CreateBuffer(uint32_t size, CmBuffer *&buffer)
{
    if ((size < CM_MIN_SURF_WIDTH) || (size > CM_MAX_1D_SURF_WIDTH))
    {
        CmAssert(0);
        return CM_INVALID_WIDTH;
    }

    CM_CREATEBUFFER_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.iSize = size;
    inParam.bufferType = CM_BUFFER_N;
    inParam.pSysMem = nullptr;
    int32_t hr = m_device->OSALExtensionExecute(CM_FN_CMDEVICE_CREATEBUFFER,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);
    buffer = (CmBuffer *)inParam.pCmBufferHandle;

    return CM_SUCCESS;
}

int32_t CmSurfaceManager::CreateBufferUP(uint32_t size,
                                     void *sysMem,
                                     CmBufferUP *&buffer)
{
    if ((size < CM_MIN_SURF_WIDTH) || (size > CM_MAX_1D_SURF_WIDTH))
    {
        CmAssert(0);
        return CM_INVALID_WIDTH;
    }
    if (sysMem == nullptr)
    {
        CmAssert(0);
        return CM_INVALID_ARG_VALUE;
    }

    CM_CREATEBUFFER_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.iSize = size;
    inParam.bufferType = CM_BUFFER_UP;
    inParam.pSysMem = sysMem;
    int32_t hr = m_device->OSALExtensionExecute(CM_FN_CMDEVICE_CREATEBUFFER,
                                                &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);

    buffer = (CmBufferUP *)inParam.pCmBufferHandle;
    return CM_SUCCESS;
}

int32_t CmSurfaceManager::DestroyBuffer(CmBuffer *&buffer)
{
    CM_DESTROYBUFFER_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_DESTROYBUFFER_PARAM));
    inParam.pCmBufferHandle = buffer;

    int32_t hr = m_device->OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYBUFFER,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);
    buffer = nullptr;

    return hr;
}

int32_t CmSurfaceManager::DestroyBufferUP(CmBufferUP *&buffer)
{
    CM_DESTROYBUFFER_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_DESTROYBUFFER_PARAM));
    inParam.pCmBufferHandle = buffer;

    int32_t hr = m_device->OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYBUFFERUP,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);
    buffer = nullptr;

    return hr;
}

int32_t CmSurfaceManager::CreateSurface2DUP(uint32_t width,
                                        uint32_t height,
                                        CM_SURFACE_FORMAT format,
                                        void *sysMem,
                                        CmSurface2DUP *&surface)
{
    int32_t result = Surface2DSanityCheck(width, height, format);
    if (result != CM_SUCCESS)
    {
        CmAssert(0);
        return result;
    }
    if (nullptr == sysMem)
    {
        CmAssert(0);
        return CM_INVALID_ARG_VALUE;
    }

    CM_CREATESURFACE2DUP_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_CREATESURFACE2DUP_PARAM));
    inParam.iWidth = width;
    inParam.iHeight = height;
    inParam.Format = format;
    inParam.pSysMem = sysMem;
    int32_t hr =
        m_device->OSALExtensionExecute(CM_FN_CMDEVICE_CREATESURFACE2DUP,
                                       &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);
    surface = (CmSurface2DUP *)inParam.pCmSurface2DUPHandle;

    return hr;
}

int32_t CmSurfaceManager::DestroySurface2DUP(CmSurface2DUP *&surface)
{
    CM_DESTROYSURFACE2DUP_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_DESTROYSURFACE2DUP_PARAM));
    inParam.pCmSurface2DUPHandle = surface;
    int32_t hr =
        m_device->OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYSURFACE2DUP,
                                       &inParam, sizeof(inParam));

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);
    surface = nullptr;
    return hr;
}

int32_t CmSurfaceManager::CreateSurface3D(uint32_t width,
                                      uint32_t height,
                                      uint32_t depth,
                                      CM_SURFACE_FORMAT format,
                                      CmSurface3D *&surface)
{
    CM_CREATE_SURFACE3D_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_CREATE_SURFACE3D_PARAM));

    inParam.iWidth = width;
    inParam.iHeight = height;
    inParam.iDepth = depth;
    inParam.Format = format;
    int32_t hr = m_device->OSALExtensionExecute(CM_FN_CMDEVICE_CREATESURFACE3D,
                                                &inParam, sizeof(inParam),
                                                nullptr, 0);

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);
    surface = (CmSurface3D *)inParam.pCmSurface3DHandle;

    return hr;
}

int32_t CmSurfaceManager::DestroySurface3D(CmSurface3D *&surface)
{
    CM_DESTROY_SURFACE3D_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_DESTROY_SURFACE3D_PARAM));
    inParam.pCmSurface3DHandle = surface;

    int32_t hr = m_device->OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYSURFACE3D,
                                                &inParam, sizeof(inParam),
                                                nullptr, 0);

    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);
    surface = nullptr;

    return hr;
}

int32_t CmSurfaceManager::CreateBufferSVM(uint32_t size,
                                      void *&sysMem,
                                      uint32_t accessFlag,
                                      CmBufferSVM *&buffer)
{
    CM_CREATEBUFFER_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.iSize = size;
    inParam.bufferType = CM_BUFFER_SVM;
    inParam.pSysMem = sysMem;
    int32_t hr = m_device->OSALExtensionExecute(CM_FN_CMDEVICE_CREATEBUFFER,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);

    buffer = (CmBufferSVM *)inParam.pCmBufferHandle;
    sysMem = inParam.pSysMem;  //Update pSystem

    return hr;
}

int32_t CmSurfaceManager::DestroyBufferSVM(CmBufferSVM *&buffer)
{
    CM_DESTROYBUFFER_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_DESTROYBUFFER_PARAM));
    inParam.pCmBufferHandle = buffer;

    int32_t hr = m_device->OSALExtensionExecute(CM_FN_CMDEVICE_DESTROYBUFFERSVM,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);

    buffer = nullptr;

    return hr;
}
