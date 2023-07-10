/*
* Copyright (c) 2009-2019, Intel Corporation
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
//! \file     media_libva_common.h
//! \brief    libva(and its extension) interface implemantation common head file
//! \details  libva(and its extension) interface implemantation common head file
//!

#ifndef __MEDIA_LIBVA_COMMON_H__
#define __MEDIA_LIBVA_COMMON_H__

#include <pthread.h>

#include "mos_context_next.h"
#include "mos_gpucontextmgr_next.h"
#include "mos_cmdbufmgr_next.h"

class CmdBufMgr;

#include "mos_os.h"
#include "mos_auxtable_mgr.h"

#ifdef _MANUAL_SOFTLET_
#include "ddi_media_functions.h"
#include "media_interfaces_hwinfo.h"
#endif

#include <va/va.h>
#include <va/va_backend.h>
#include <va/va_backend_vpp.h>
#include <va/va_drmcommon.h>
#include <va/va_dec_jpeg.h>
#include <va/va_backend.h>

#include "media_libva_common_next.h"
#ifdef ANDROID
#include <utils/Log.h>

#ifndef LOG_TAG
#define LOG_TAG "DDI"
#endif
#if ANDROID_VERSION > 439 && defined(ENABLE_ATRACE)
#ifndef HAVE_ANDROID_OS
#define HAVE_ANDROID_OS
#endif
#define ATRACE_TAG                      (ATRACE_TAG_VIDEO | ATRACE_TAG_HAL)
#include <cutils/trace.h>
#define UMD_ATRACE_BEGIN(name)              \
{                                           \
    if(atrace_switch) ATRACE_BEGIN(name);   \
}
#define UMD_ATRACE_END                      \
{                                           \
    if(atrace_switch) ATRACE_END();         \
}
#include <cutils/properties.h>
static int32_t atrace_switch            = 0;
#else
#define UMD_ATRACE_BEGIN                __noop
#define UMD_ATRACE_END                  __noop
#endif
#else
#define UMD_ATRACE_BEGIN                __noop
#define UMD_ATRACE_END                  __noop
#endif

#define DDI_UNUSED(param)                      MOS_UNUSED(param)

// heap
#define DDI_MEDIA_HEAP_INCREMENTAL_SIZE      8


#define DDI_MEDIA_VACONTEXTID_OFFSET_MFE           0x70000000
#define DDI_MEDIA_VACONTEXTID_OFFSET_CM            0x80000000

#ifndef VA_FOURCC_ABGR
#define VA_FOURCC_ABGR          VA_FOURCC('A', 'B', 'G', 'R')
#endif

#ifndef VA_FOURCC_R5G6B5
#define VA_FOURCC_R5G6B5        VA_FOURCC('R','G','1', '6')
#endif

#ifndef VA_FOURCC_R8G8B8
#define VA_FOURCC_R8G8B8        VA_FOURCC('R','G','2', '4')
#endif

#ifndef VA_FOURCC_I420
#define VA_FOURCC_I420        VA_FOURCC('I','4','2', '0')
#endif

#define RGB_10BIT_ALPHAMASK     VA_RT_FORMAT_RGB32_10BPP
#define RGB_8BIT_ALPHAMASK      0

#define MEDIAAPI_EXPORT __attribute__((visibility("default")))

class MediaLibvaCaps;
class MediaLibvaCapsNext;

#include "ddi_media_context.h"
typedef struct DDI_MEDIA_CONTEXT *PDDI_MEDIA_CONTEXT;

static __inline PDDI_MEDIA_CONTEXT DdiMedia_GetMediaContext (VADriverContextP ctx)
{
    return (PDDI_MEDIA_CONTEXT)ctx->pDriverData;
}

//!
//! \brief  Media surface to mos resource
//!
//! \param  [in] mediaSurface
//!     Ddi media surface
//! \param  [in] mhalOsResource
//!     Mos resource
//!
void DdiMedia_MediaSurfaceToMosResource(DDI_MEDIA_SURFACE *mediaSurface, MOS_RESOURCE  *mhalOsResource);

//!
//! \brief  Media buffer to mos resource
//!
//! \param  [in] mediaBuffer
//!     Ddi media buffer
//! \param  [in] mhalOsResource
//!     Mos resource
//!
void DdiMedia_MediaBufferToMosResource(DDI_MEDIA_BUFFER *mediaBuffer, MOS_RESOURCE  *mhalOsResource);

//!
//! \brief  Get context from context ID
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] vaCtxID
//!     VA context ID
//! \param  [in] ctxType
//!     Ctx type
//!
void* DdiMedia_GetContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID, uint32_t *ctxType);

//!
//! \brief  Get surface from VA surface ID
//!
//! \param  [in] mediaCtx
//!     Pointer to ddi media context
//! \param  [in] surfaceID
//!     VA surface ID
//!
//! \return DDI_MEDIA_SURFACE*
//!     Pointer to ddi media surface
//!
DDI_MEDIA_SURFACE* DdiMedia_GetSurfaceFromVASurfaceID (PDDI_MEDIA_CONTEXT mediaCtx, VASurfaceID surfaceID);

//!
//! \brief  replace the surface with given format
//!
//! \param  [in] surface
//!     Pointer to the old surface
//! \param  [in] expectedFormat
//!     VA surface ID
//!
//! \return DDI_MEDIA_SURFACE*
//!     Pointer to new ddi media surface
//!
PDDI_MEDIA_SURFACE DdiMedia_ReplaceSurfaceWithNewFormat(PDDI_MEDIA_SURFACE surface, DDI_MEDIA_FORMAT expectedFormat);

//!
//! \brief  replace the surface with correlation variant format
//!
//! \param  [in] surface
//!     Pointer to the old surface
//!
//! \return DDI_MEDIA_SURFACE*
//!     Pointer to new ddi media surface
//!
PDDI_MEDIA_SURFACE DdiMedia_ReplaceSurfaceWithVariant(PDDI_MEDIA_SURFACE surface, VAEntrypoint entrypoint);

//!
//! \brief  Get VA surface ID  from surface
//!
//! \param  [in] surface
//!     surface
//!
//! \return VASurfaceID
//!     VA Surface ID
//!
VASurfaceID DdiMedia_GetVASurfaceIDFromSurface(PDDI_MEDIA_SURFACE surface);

//!
//! \brief  Get buffer from VA buffer ID
//!
//! \param  [in] mediaCtx
//!     Pointer to ddi media context
//! \param  [in] bufferID
//!     VA buffer ID
//!
//! \return DDI_MEDIA_BUFFER*
//!     Pointer to ddi media buffer
//!
DDI_MEDIA_BUFFER* DdiMedia_GetBufferFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID);

//!
//! \brief  Get context from VA buffer ID
//!
//! \param  [in] mediaCtx
//!     Pointer to ddi media context
//! \param  [in] bufferID
//!     VA buffer ID
//!
//! \return void*
//!     Pointer to context
//!
void* DdiMedia_GetContextFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID);

//!
//! \brief  Destroy buffer from VA buffer ID
//!
//! \param  [in] mediaCtx
//!     Pointer to ddi media context
//! \param  [in] bufferID
//!     VA buffer ID
//!
//! \return     bool
//!     true if destroy buffer from VA buffer ID, else false
//!
bool DdiMedia_DestroyBufFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID);

//!
//! \brief  Get gpu priority
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] buffers
//!     VA buffer ID
//! \param  [in] numBuffers
//!     Number of buffers
//! \param  [out] updatePriority
//!     Update priority
//! \param  [out] priority
//!     Priority value
//! \return     int32_t
//!
int32_t DdiMedia_GetGpuPriority (VADriverContextP ctx, VABufferID *buffers, int32_t numBuffers, bool *updatePriority, int32_t *priority);

//!
//! \brief  Move a bufferID to the end of buffers
//!
//! \param  [in,out] buffers
//!     VA buffer ID
//! \param  [in] priorityIndexInBuf
//!     Location of priority buffer
//! \param  [in] numBuffers
//!     Number of buffers
//! \return     void
//!
void MovePriorityBufferIdToEnd (VABufferID *buffers, int32_t priorityIndexInBuf, int32_t numBuffers);

#endif
