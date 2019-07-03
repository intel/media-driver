/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file      media_libva_cm.c 
//! \brief     LibVA CM(C for Metal) extensions interface implementaion 
//!
#include <va/va.h>
#include <va/va_vpp.h>
#include <va/va_backend.h>

#include <dlfcn.h>

#include "vphal.h"

#include "media_libva_cm.h"
#include "media_libva_util.h"
#include "cm_debug.h"
#include "cm_wrapper_os.h"

/////////////////////////////////////////////////////////////////////////////
//! \purpose: Free CM context 
//! \params
//! [in]
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////
VAStatus DdiDestroyContextCM (
    VADriverContextP    vaDriverCtx,
    VAContextID         vaCtxID)
{
    DDI_UNUSED(vaDriverCtx);
    DDI_UNUSED(vaCtxID);
    return VA_STATUS_SUCCESS;
}

#define CTX(dpy) (((VADisplayContextP)dpy)->pDriverContext)
/////////////////////////////////////////////////////////////////////////////
//! \purpose: communcation functions between CM Runtime and CM context in VA 
//! \this function will be dynamical opened as a library by vaGetLibFunc() called by CMRT 
//! \params
//! [in]  VADisplay 
//! [in]  CM module type
//! [in]  input function ID 
//! [in]  input parameter data
//! [in]  input parameter size 
//! [in]  Size of the private data
//! [out] output function ID 
//! [out] output parameter data
//! [out] output parameter size 
//! returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////
VAStatus vaCmExtSendReqMsg(
     VADisplay display,
     void      *moduleType,
     uint32_t  *inputFunId,
     void      *inputData,
     uint32_t  *inputDataLen,
     uint32_t  *outputFunId,
     void      *outputData,
     uint32_t  *outputDataLen)
{
    VADriverContextP vaDriverCtx;
    VAStatus         hr;
    int32_t          funcID;
    VAContextID      vaCtxID;
    void *           deviceHandle;
    DDI_UNUSED(outputFunId);
    DDI_UNUSED(outputDataLen);

    CM_FUNCTION_ENTER_DDI;

    hr        = VA_STATUS_ERROR_UNKNOWN;
    CM_CHK_NULL_RETURN_WITH_MSG(display, VA_STATUS_ERROR_INVALID_PARAMETER, "Null VADisplay!");
    vaDriverCtx = CTX(display);
    CM_CHK_NULL_RETURN_WITH_MSG(vaDriverCtx, VA_STATUS_ERROR_INVALID_PARAMETER, "Null vaDriverCtx!");
    funcID    = *(int *)inputFunId;

    deviceHandle  = outputData;
    if ( *(int *)moduleType != VAExtModuleCMRT)
    {
        return VA_STATUS_ERROR_UNKNOWN;
    }

    hr = CmThinExecute(vaDriverCtx, deviceHandle, *inputFunId, inputData, *inputDataLen);
    if(hr != VA_STATUS_SUCCESS)
    {
        CM_ASSERTMESSAGE_DDI("CmThinExecute Failed FunctionID %x, ret %d \n", *inputFunId, hr);
    }

    return hr;
}
