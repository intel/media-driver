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
//! \file      media_libva_util.h 
//! \brief     libva(and its extension) utility head file  
//!
#ifndef __MEDIA_LIBVA_UTIL_H__
#define __MEDIA_LIBVA_UTIL_H__

#include "media_libva_common.h"
#include "mos_util_debug.h"
#include "mos_bufmgr.h"

#define DEVICE_NAME "/dev/dri/renderD128"   // For Gen, it is always /dev/dri/renderD128 node

void     DdiMediaUtil_MediaDumpNV12(uint8_t *pSrcY, uint8_t *pSrcUV, int32_t iWidth, int32_t iHeight, int32_t iPitch, int32_t iFrameNum);
void     DdiMediaUtil_MediaPrintFps();
VAStatus DdiMediaUtil_CreateSurface(DDI_MEDIA_SURFACE  *pSurface, PDDI_MEDIA_CONTEXT pMediaDrvCtx);
VAStatus DdiMediaUtil_CreateBuffer(DDI_MEDIA_BUFFER *pBuffer, mos_bufmgr *pBufmgr);

void*    DdiMediaUtil_LockSurface(DDI_MEDIA_SURFACE  *pSurface, uint32_t flag);
void     DdiMediaUtil_UnlockSurface(DDI_MEDIA_SURFACE  *pSurface);

void*    DdiMediaUtil_LockBuffer(DDI_MEDIA_BUFFER *pBuf, uint32_t flag);
void     DdiMediaUtil_UnlockBuffer(DDI_MEDIA_BUFFER *pBuf);

void     DdiMediaUtil_FreeSurface(DDI_MEDIA_SURFACE *pSurface);
void     DdiMediaUtil_FreeBuffer(DDI_MEDIA_BUFFER  *pBuf);

void     DdiMediaUtil_InitMutex(PMEDIA_MUTEX_T  pMutex);
void     DdiMediaUtil_DestroyMutex(PMEDIA_MUTEX_T  pMutex);
void     DdiMediaUtil_LockMutex(PMEDIA_MUTEX_T  pMutex);
void     DdiMediaUtil_UnLockMutex(PMEDIA_MUTEX_T  pMutex);
void     DdiMediaUtil_InitSemaphore(PMEDIA_SEM_T  pSem, uint32_t uiInitCount);
void     DdiMediaUtil_DestroySemaphore(PMEDIA_SEM_T  pSem);
void     DdiMediaUtil_WaitSemaphore(PMEDIA_SEM_T  pSem);
int32_t  DdiMediaUtil_TryWaitSemaphore(PMEDIA_SEM_T  pSem);
void     DdiMediaUtil_PostSemaphore(PMEDIA_SEM_T  pSem);

VAStatus DdiMediaUtil_ConvertBufImageToSurface(DDI_MEDIA_BUFFER *pBuf, VAImage *pImage, DDI_MEDIA_SURFACE *pSurface);
VAStatus DdiMediaUtil_FillPositionToRect(RECT *rect, int16_t offset_x, int16_t offset_y, int16_t width, int16_t height);
bool     DdiMediaUtil_IsExternalSurface(PDDI_MEDIA_SURFACE pSurface);

PDDI_MEDIA_SURFACE_HEAP_ELEMENT DdiMediaUtil_AllocPMediaSurfaceFromHeap(PDDI_MEDIA_HEAP pSurfaceHeap);
void     DdiMediaUtil_ReleasePMediaSurfaceFromHeap(PDDI_MEDIA_HEAP pSurfaceHeap, uint32_t uiVaSurfaceID);

PDDI_MEDIA_BUFFER_HEAP_ELEMENT  DdiMediaUtil_AllocPMediaBufferFromHeap(PDDI_MEDIA_HEAP pBufferHeap);
void     DdiMediaUtil_ReleasePMediaBufferFromHeap(PDDI_MEDIA_HEAP pBufferHeap, uint32_t uiVaBufferID);

PDDI_MEDIA_IMAGE_HEAP_ELEMENT  DdiMediaUtil_AllocPVAImageFromHeap(PDDI_MEDIA_HEAP pImageHeap);
void     DdiMediaUtil_ReleasePVAImageFromHeap(PDDI_MEDIA_HEAP pImageHeap, uint32_t uiVAImageID);

PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT DdiMediaUtil_AllocPVAContextFromHeap(PDDI_MEDIA_HEAP pVaContextHeap);
void     DdiMediaUtil_ReleasePVAContextFromHeap(PDDI_MEDIA_HEAP pVaContextHeap, uint32_t uiVAContextID);


void     DdiMediaUtil_UnRefBufObjInMediaBuffer(PDDI_MEDIA_BUFFER pBuf);
void     DdiMediaUtil_GetEnabledFeature(PDDI_MEDIA_CONTEXT pMediaDrvCtx);
int32_t  DdiMediaUtil_OpenGraphicsAdaptor(char *pDevName);

//------------------------------------------------------------------------------
// Macros for debug messages, Assert, Null check and condition check within ddi files
//------------------------------------------------------------------------------

#define DDI_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _expr)

#define DDI_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define DDI_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define DDI_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#ifdef ANDROID
#define DDI_FUNCTION_ENTER()            UMD_ATRACE_BEGIN(__FUNCTION__)
#define DDI_FUNCTION_EXIT(status)       UMD_ATRACE_END
#else
#define DDI_FUNCTION_ENTER()                                                \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF)

#define DDI_FUNCTION_EXIT(status)                                               \
    MOS_FUNCTION_EXIT(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, status)
#endif

// If pointer is nullptr, print the error message and return the specified value.
#define DDI_CHK_NULL(_ptr, _str, _ret)                                      \
    DDI_CHK_CONDITION((nullptr == (_ptr)), _str, _ret)

#define DDI_CHK_LARGER(p, bottom, str, ret)                                 \
    DDI_CHK_CONDITION((p <= bottom),str,ret)
    
#define DDI_CHK_LESS(p, upper, str, ret)                                    \
    DDI_CHK_CONDITION((p >= upper),str,ret)

// Check the return value of function.
// If failed,  print the error message and return,
// do nothing otherwise.
#define DDI_CHK_RET(_ret, _str)                                           \
{                                                                             \
    VAStatus tmpRet = _ret;                                                   \
    if (VA_STATUS_SUCCESS != tmpRet) {                                        \
        DDI_ASSERTMESSAGE("%s [%d].", _str, tmpRet);                        \
        return tmpRet;                                                        \
    }                                                                         \
}

// Check the return status of parse function in renderPicture
// If failed, assign new status and break
// do nothing otherwise.
#define DDI_CHK_STATUS(_ret, _newret)                                       \
{                                                                             \
    if (VA_STATUS_SUCCESS != _ret) {                                          \
        vaStatus = _newret;                                                   \
        break;                                                                \
    }                                                                         \
}

// Check the condition, if true, print the error message 
// and return the specified value, do nothing otherwise.
#define DDI_CHK_CONDITION(condition, _str, _ret)                            \
    if (condition) {                                                          \
        DDI_ASSERTMESSAGE(_str);                                            \
        return _ret;                                                          \
    }

#define DDI_CHK_STATUS_MESSAGE(_ptr, _message, ...)                         \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _ptr, _message, ##__VA_ARGS__)

#define DDI_CHK_HR_MESSAGE(_ptr, _message, ...)                             \
    MOS_CHK_HR_MESSAGE(MOS_COMPONENT_DDI, MOS_DDI_SUBCOMP_SELF, _ptr, _message, ##__VA_ARGS__)

#ifdef ANDROID
// Enable new specification for video formats
// \see memo "Color format Usage on Android"
#ifndef UFO_GRALLOC_NEW_FORMAT
#define UFO_GRALLOC_NEW_FORMAT 1
#endif
#endif

#endif
