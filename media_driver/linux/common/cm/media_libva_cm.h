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
//! \file      media_libva_cm.h
//! \brief     LibVA CM(C for Metal) extensions interface definition
//!

#ifndef _MEDIA_LIBVA_CM_H_
#define _MEDIA_LIBVA_CM_H_

#include "media_libva_common.h"
#include <va/va.h>
#include <va/va_vpp.h>
#include <va/va_backend_vpp.h>

#include "vphal.h"
#include "mos_os.h"

#define VAExtModuleCMRT 2

//core structure for CM DDI
struct _CM_HAL_STATE;
typedef struct _CM_HAL_STATE *PCM_HAL_STATE;
typedef struct _CM_CONTEXT
{
    MOS_CONTEXT  mosCtx;
    union
    {
        void                  *cmHal;
        PCM_HAL_STATE         cmHalState;
    };
} CM_CONTEXT, *PCM_CONTEXT;

#ifdef __cplusplus
extern "C" {
#endif
//Public APIs to access CM acceleration capability in VPG drivers
VAStatus DdiDestroyContextCM (
    VADriverContextP    vaDriverCtx,
    VAContextID         vaCtxID);

MEDIAAPI_EXPORT VAStatus vaCmExtSendReqMsg(
     VADisplay dpy,
     void      *moduleType,
     uint32_t  *inputFunId,
     void      *inputData,
     uint32_t  *inputDataLen,
     uint32_t  *outputFunId,
     void      *outputData,
     uint32_t  *outputDataLen);

uint32_t CmGetFreeCtxIndex (
    PDDI_MEDIA_CONTEXT mediaCtx);

#ifdef __cplusplus
}
#endif
#endif //_MEDIA_LIBVA_CM_H_
