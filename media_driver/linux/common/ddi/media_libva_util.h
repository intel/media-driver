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
#include "mos_bufmgr_api.h"

#define DEVICE_NAME "/dev/dri/renderD128"   // For Gen, it is always /dev/dri/renderD128 node

//!
//! \brief  Media print frame per second
//!
void     DdiMediaUtil_MediaPrintFps();

//!
//! \brief  Create surface
//! 
//! \param  [in] surface
//!         Ddi media surface
//! \param  [in] mediaDrvCtx
//!         Pointer to ddi media context
//!         
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMediaUtil_CreateSurface(DDI_MEDIA_SURFACE  *surface, PDDI_MEDIA_CONTEXT mediaDrvCtx);

//!
//! \brief  Create buffer
//! 
//! \param  [out] buffer
//!         Ddi media buffer
//! \param  [in] bufmgr
//!         Mos buffer manager
//!         
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMediaUtil_CreateBuffer(DDI_MEDIA_BUFFER *buffer, mos_bufmgr *bufmgr);

//!
//! \brief  Lock surface
//!
//! \param  [in] surface
//!         Ddi media surface
//! \param  [in] flag
//!         Flag
//!
//! \return void*
//!     Pointer to lock surface data
//!
void*    DdiMediaUtil_LockSurface(DDI_MEDIA_SURFACE  *surface, uint32_t flag);

//!
//! \brief  Lock surface
//!
//! \param  [in] surface
//!         Ddi media surface
//! \param  [in] flag
//!         Flag
//!
//! \return void*
//!     Pointer to lock surface data
//!
void* DdiMediaUtil_LockSurfaceInternal(DDI_MEDIA_SURFACE *surface, uint32_t flag);

//!
//! \brief  Unlock surface
//!
//! \param  [in] surface
//!         Ddi media surface
//!
void     DdiMediaUtil_UnlockSurface(DDI_MEDIA_SURFACE  *surface);

//!
//! \brief  Unlock surface
//!
//! \param  [in] surface
//!         Ddi media surface
//!
void     DdiMediaUtil_UnlockSurfaceInternal(DDI_MEDIA_SURFACE *surface);

//!
//! \brief  Lock buffer
//! 
//! \param  [in] buf
//!         Ddi media buffer
//! \param  [in] flag
//!         Flag
//!         
//! \return void*
//!     Pointer to lock buffer data
//!
void*    DdiMediaUtil_LockBuffer(DDI_MEDIA_BUFFER *buf, uint32_t flag);

//!
//! \brief  Unlock buffer
//! 
//! \param  [in] buf
//!         Ddi media buffer
//!
void     DdiMediaUtil_UnlockBuffer(DDI_MEDIA_BUFFER *buf);

//!
//! \brief  Free surface
//! 
//! \param  [in] surface
//!         Ddi media surface
//!
void     DdiMediaUtil_FreeSurface(DDI_MEDIA_SURFACE *surface);

//!
//! \brief  Free buffer
//! 
//! \param  [in] buf
//!         Ddi media buffer
//!
void     DdiMediaUtil_FreeBuffer(DDI_MEDIA_BUFFER  *buf);

//!
//! \brief  Init mutex
//! 
//! \param  [in] mutex
//!         Pointer to media mutex thread
//!
void     DdiMediaUtil_InitMutex(PMEDIA_MUTEX_T  mutex);

//!
//! \brief  Destroy mutex
//! 
//! \param  [in] mutex
//!         Pointer to media mutex thread
//!
void     DdiMediaUtil_DestroyMutex(PMEDIA_MUTEX_T  mutex);
//!
//! \brief  Lock mutex
//! 
//! \param  [in] mutex
//!         Pointer to media mutex thread
//!
void     DdiMediaUtil_LockMutex(PMEDIA_MUTEX_T  mutex);

//!
//! \brief  Unlock mutex
//! 
//! \param  [in] mutex
//!         Pointer to media mutex thread
//!
void     DdiMediaUtil_UnLockMutex(PMEDIA_MUTEX_T  mutex);

//!
//! \brief  Helper inline class intended to simplify mutex lock/unlock
//!         operations primarily used as a stack-allocated object.
//!         In that case, the compiler guarantees to call the destructor
//!         leaving the scope. The class becomes handy in functions
//!         where there are several return statements with different
//!         exit code value.
//!
class DdiMediaUtil_LockGuard {
private:
    PMEDIA_MUTEX_T m_pMutex;
public:
    DdiMediaUtil_LockGuard(PMEDIA_MUTEX_T pMutex):m_pMutex(pMutex)
    {
        DdiMediaUtil_LockMutex(m_pMutex);
    }
    ~DdiMediaUtil_LockGuard()
    {
        DdiMediaUtil_UnLockMutex(m_pMutex);
    }
};

//!
//! \brief  Destroy semaphore
//! 
//! \param  [in] sem
//!         Pointer to media semaphore thread
//!
void     DdiMediaUtil_DestroySemaphore(PMEDIA_SEM_T  sem);

//!
//! \brief  Wait semaphore
//! 
//! \param  [in] sem
//!         Pointer to media semaphore thread
//!
void     DdiMediaUtil_WaitSemaphore(PMEDIA_SEM_T  sem);
//!
//! \brief  Try wait semaphore
//! 
//! \param  [in] sem
//!         Pointer to media semaphore thread
//!
//! \return int32_t 
//!     Try wait for semaphore. Return 0 if success, else -1 if fail        
//!
int32_t  DdiMediaUtil_TryWaitSemaphore(PMEDIA_SEM_T  sem);

//!
//! \brief  Post semaphore
//! 
//! \param  [in] sem
//!         Pointer to media semaphore thread
//!
void     DdiMediaUtil_PostSemaphore(PMEDIA_SEM_T  sem);

//!
//! \brief  Fill a rect structure with the regsion specified by parameters
//! 
//! \param  [in] rect
//!         Input pointer to the rect
//! \param  [in] offset_x
//!         X offset of the region
//! \param  [in] offset_y
//!         Y offset of the region
//! \param  [in] width
//!         Width of the region
//! \param  [in] height
//!         Height of the region
//!         
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMediaUtil_FillPositionToRect(RECT *rect, int16_t offset_x, int16_t offset_y, int16_t width, int16_t height);

//!
//! \brief  Is external surface
//! \details    If the bo of media surface was allocated from App,
//!     should return true, otherwise, false. In current implemeation
//!     external buffer passed with pSurfDesc.
//! 
//! \param  [in] surface
//!         Pointer to ddi media surface
//!         
//! \return bool
//!     true if surface is external, else false
//!
bool     DdiMediaUtil_IsExternalSurface(PDDI_MEDIA_SURFACE surface);

//!
//! \brief  Allocate pmedia surface from heap
//! 
//! \param  [in] surfaceHeap
//!         Pointer to ddi media heap
//!         
//! \return PDDI_MEDIA_SURFACE_HEAP_ELEMENT
//!     Pointer to ddi media surface heap element
//!
PDDI_MEDIA_SURFACE_HEAP_ELEMENT DdiMediaUtil_AllocPMediaSurfaceFromHeap(PDDI_MEDIA_HEAP surfaceHeap);

//!
//! \brief  Release pmedia surface from heap
//! 
//! \param  [in] surfaceHeap
//!         Pointer to ddi media heap
//! \param  [in] vaSurfaceID
//!         VA surface ID
//!
void     DdiMediaUtil_ReleasePMediaSurfaceFromHeap(PDDI_MEDIA_HEAP surfaceHeap, uint32_t vaSurfaceID);

//!
//! \brief  Allocate pmedia buffer from heap
//! 
//! \param  [in] bufferHeap
//!         Pointer to ddi media heap
//!         
//! \return PDDI_MEDIA_BUFFER_HEAP_ELEMENT
//!     Pointer to ddi media buffer heap element
//!
PDDI_MEDIA_BUFFER_HEAP_ELEMENT  DdiMediaUtil_AllocPMediaBufferFromHeap(PDDI_MEDIA_HEAP bufferHeap);

//!
//! \brief  Release pmedia buffer from heap
//! 
//! \param  [in] bufferHeap
//!         Pointer to ddi media heap
//! \param  [in] vaBufferID
//!         VA buffer ID
//!
void     DdiMediaUtil_ReleasePMediaBufferFromHeap(PDDI_MEDIA_HEAP bufferHeap, uint32_t vaBufferID);

//!
//! \brief  Allocate PVA image from heap
//! 
//! \param  [in] imageHeap
//!         Pointer to ddi media heap
//!         
//! \return PDDI_MEDIA_IMAGE_HEAP_ELEMENT
//!     Pointer to ddi media image heap element
//!
PDDI_MEDIA_IMAGE_HEAP_ELEMENT  DdiMediaUtil_AllocPVAImageFromHeap(PDDI_MEDIA_HEAP imageHeap);

//!
//! \brief  Release PVA image from heap
//! 
//! \param  [in] imageHeap
//!         Pointer to ddi media heap
//! \param  [in] vaImageID
//!         VA image ID
//!
void     DdiMediaUtil_ReleasePVAImageFromHeap(PDDI_MEDIA_HEAP imageHeap, uint32_t vaImageID);

//!
//! \brief  Allocate PVA context from heap
//! 
//! \param  [in] vaContextHeap
//!         Pointer to ddi media heap
//!         
//! \return PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT
//!     Pointer to ddi media vacontext heap element
//!
PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT DdiMediaUtil_AllocPVAContextFromHeap(PDDI_MEDIA_HEAP vaContextHeap);

//!
//! \brief  Release PVA context from heap
//! 
//! \param  [in] vaContextHeap
//!         Pointer to ddi media heap
//! \param  [in] vaContextID
//!         VA context ID
//!
void     DdiMediaUtil_ReleasePVAContextFromHeap(PDDI_MEDIA_HEAP vaContextHeap, uint32_t vaContextID);

//!
//! \brief  Unreference buf object media buffer
//! 
//! \param  [in] buf
//!         Pointer to ddi media buffer
//!
void     DdiMediaUtil_UnRefBufObjInMediaBuffer(PDDI_MEDIA_BUFFER buf);

//!
//! \brief  Open Intel's Graphics Device to get the file descriptor
//! 
//! \param  [in] devName
//!         Device name
//!         
//! \return int32_t 
//!     Device name header. Return 0 if success, else -1 if fail.
//!
int32_t  DdiMediaUtil_OpenGraphicsAdaptor(char *devName);

//!
//! \brief  Unregister RT surfaces
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] surface
//!     Pointer to ddi media surface
//!     
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMediaUtil_UnRegisterRTSurfaces(VADriverContextP    ctx,PDDI_MEDIA_SURFACE surface);

//!
//! \brief  Determine whethere media reset is anabled
//!
//! \param  [in] mediaCtx
//!     Pointer to VA driver context
//!
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMediaUtil_SetMediaResetEnableFlag(PDDI_MEDIA_CONTEXT mediaCtx);

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
