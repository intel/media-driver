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

#include "xf86drm.h"
#include "drm.h"
#include "i915_drm.h"
#include "mos_bufmgr.h"
#include "mos_context.h"
#include "mos_gpucontextmgr.h"
#include "mos_cmdbufmgr.h"

#include "mos_context_next.h"
#include "mos_gpucontextmgr_next.h"
#include "mos_cmdbufmgr_next.h"

#include "mos_os.h"
#include "mos_auxtable_mgr.h"

#include <va/va.h>
#include <va/va_backend.h>
#include <va/va_backend_vpp.h>
#include <va/va_drmcommon.h>
#include <va/va_dec_jpeg.h>
#include <va/va_backend.h>

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

#define DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT   127
#define DDI_MEDIA_MAX_INSTANCE_NUMBER          0x0FFFFFFF

// heap
#define DDI_MEDIA_HEAP_INCREMENTAL_SIZE      8

#define DDI_MEDIA_VACONTEXTID_OFFSET_DECODER       0x10000000
#define DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER       0x20000000
#define DDI_MEDIA_VACONTEXTID_OFFSET_CENC          0x30000000
#define DDI_MEDIA_VACONTEXTID_OFFSET_VP            0x40000000
#define DDI_MEDIA_VACONTEXTID_OFFSET_MFE           0x70000000
#define DDI_MEDIA_VACONTEXTID_OFFSET_CM            0x80000000
#define DDI_MEDIA_MASK_VACONTEXT_TYPE              0xF0000000

#define DDI_MEDIA_MASK_VACONTEXTID                 0x0FFFFFFF
#define DDI_MEDIA_CONTEXT_TYPE_DECODER             1
#define DDI_MEDIA_CONTEXT_TYPE_ENCODER             2
#define DDI_MEDIA_CONTEXT_TYPE_VP                  3
#define DDI_MEDIA_CONTEXT_TYPE_MEDIA               4
#define DDI_MEDIA_CONTEXT_TYPE_CM                  5
#define DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER        6
#define DDI_MEDIA_CONTEXT_TYPE_MFE                 7
#define DDI_MEDIA_CONTEXT_TYPE_NONE                0

#define DDI_MEDIA_INVALID_VACONTEXTID              0

#define DDI_MEDIA_MAX_COLOR_PLANES                 4       //Maximum color planes supported by media driver, like (A/R/G/B in different planes)

typedef pthread_mutex_t  MEDIA_MUTEX_T, *PMEDIA_MUTEX_T;
#define MEDIA_MUTEX_INITIALIZER  PTHREAD_MUTEX_INITIALIZER

typedef sem_t MEDIA_SEM_T, *PMEDIA_SEM_T;

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

typedef enum _DDI_MEDIA_FORMAT
{
    Media_Format_NV12        ,
    Media_Format_NV21        ,
    Media_Format_Buffer      ,
    Media_Format_2DBuffer    ,
    Media_Format_Perf_Buffer ,
    Media_Format_X8R8G8B8    ,
    Media_Format_A8R8G8B8    ,
    Media_Format_X8B8G8R8    ,
    Media_Format_A8B8G8R8    ,
    Media_Format_R8G8B8A8    ,
    Media_Format_R5G6B5      ,
    Media_Format_R10G10B10A2 ,
    Media_Format_B10G10R10A2 ,
    Media_Format_CPU         ,

    Media_Format_YUY2        ,
    Media_Format_UYVY        ,
    Media_Format_YV12        ,
    Media_Format_IYUV        ,
    Media_Format_I420        ,

    Media_Format_422H        ,
    Media_Format_444P        ,
    Media_Format_411P        ,
    Media_Format_400P        ,
    Media_Format_422V        ,
    Media_Format_IMC3        ,

    Media_Format_P010        ,
    Media_Format_R8G8B8      ,
    Media_Format_RGBP        ,
    Media_Format_BGRP        ,

    Media_Format_P016        ,
    Media_Format_Y210        ,
    Media_Format_Y216        ,
    Media_Format_AYUV        ,
    Media_Format_Y410        ,
    Media_Format_Y416        ,
    Media_Format_Y8          ,
    Media_Format_Y16S        ,
    Media_Format_Y16U        ,
    Media_Format_VYUY        ,
    Media_Format_YVYU        ,
    Media_Format_A16R16G16B16,
    Media_Format_A16B16G16R16,
    Media_Format_Count
} DDI_MEDIA_FORMAT;

typedef enum _DDI_MEDIA_STATUS_REPORT_QUERY_STATE
{
    DDI_MEDIA_STATUS_REPORT_QUERY_STATE_INIT,
    DDI_MEDIA_STATUS_REPORT_QUERY_STATE_PENDING,
    DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED,
    DDI_MEDIA_STATUS_REPORT_QUERY_STATE_RELEASED
} DDI_MEDIA_STATUS_REPORT_QUERY_STATE;

//!
//! \brief Surface descriptor for external DRM buffer
//!
typedef struct _DDI_MEDIA_SURFACE_DESCRIPTOR
{
    uint32_t   uiPlanes;                              // brief number of planes for planar layout
    uint32_t   uiPitches[DDI_MEDIA_MAX_COLOR_PLANES]; // pitch for each plane in bytes
    uint32_t   uiOffsets[DDI_MEDIA_MAX_COLOR_PLANES]; // offset for each plane in bytes
    uintptr_t  ulBuffer;                              // buffer handle or user pointer
    uint32_t   uiSize;                                // buffer size
    uint32_t   uiFlags;                               // See "Surface external buffer descriptor flags"
    uint32_t   uiVaMemType;                           // VA Mem type
    uint32_t   uiTile;                                // Used for user pointer
    uint32_t   uiBuffserSize;                         // Used for user pointer
    bool       bIsGralloc;                            // buffer allocated by Gralloc
    void      *pPrivateData;                          // brief reserved for passing private data
    GMM_RESCREATE_PARAMS GmmParam;                    // GMM Params for Gralloc buffer
} DDI_MEDIA_SURFACE_DESCRIPTOR,*PDDI_MEDIA_SURFACE_DESCRIPTOR;

//!
//! \struct DDI_MEDIA_CONTEXT
//! \brief  Ddi media context
//!
struct DDI_MEDIA_CONTEXT;

typedef struct DDI_MEDIA_CONTEXT *PDDI_MEDIA_CONTEXT;

typedef union _DDI_MEDIA_SURFACE_STATUS_REPORT
{
    //!
    //! \struct _DDI_MEDIA_SURFACE_DECODE_STATUS
    //! \brief  Ddi media surface decode status
    //!
    struct _DDI_MEDIA_SURFACE_DECODE_STATUS
    {
        uint32_t                   status;    // indicate latest decode status for current surface, refer to CODECHAL_STATUS in CodechalDecodeStatusReport.
        uint32_t                   errMbNum;  // indicate number of MB s with decode error, refer to NumMbsAffected in CodechalDecodeStatusReport
        uint32_t                   crcValue;  // indicate the CRC value of the decoded data
    } decode;
    //!
    //! \struct _DDI_MEDIA_SURFACE_CENC_STATUS
    //! \brief  Ddi media surface cenc status
    //! 
    struct _DDI_MEDIA_SURFACE_CENC_STATUS
    {
        uint32_t                   status;    // indicate latest cenc status for current surface, refer to CODECHAL_STATUS in CodechalDecodeStatusReport.
        uint32_t                   reserved;  // reserved
    } cenc;
    //!
    //! \struct _DDI_MEDIA_SURFACE_VPP_STATUS
    //! \brief  Ddi media surface vpp status
    //!
    struct _DDI_MEDIA_SURFACE_VPP_STATUS
    {
        uint32_t                   status;    // indicate latest vpp status for current surface.
        uint32_t                   reserved;  // reserved.
    } vpp;
} DDI_MEDIA_SURFACE_STATUS_REPORT, *PDDI_MEDIA_SURFACE_STATUS_REPORT;

typedef struct _DDI_MEDIA_SURFACE
{
    // for hwcomposer, remove this after we have a solution
    uint32_t                base;
    int32_t                 iWidth;
    int32_t                 iHeight;             // allocate height after alignment
    int32_t                 iRealHeight;         // real height before alignment
    int32_t                 iPitch;
    uint32_t                uiOffset;
    DDI_MEDIA_FORMAT        format;
    uint32_t                uiLockedBufID;
    uint32_t                uiLockedImageID;
    int32_t                 iRefCount;
    uint8_t                *pData;
    uint32_t                data_size;
    uint32_t                isTiled;
    uint32_t                TileType;
    uint32_t                bMapped;
    MOS_LINUX_BO           *bo;
    uint32_t                name;
    uint32_t                surfaceUsageHint;
    PDDI_MEDIA_SURFACE_DESCRIPTOR pSurfDesc;          // nullptr means surface was allocated by media driver
                                                      // !nullptr means surface was allocated by Application
    GMM_RESOURCE_INFO      *pGmmResourceInfo;   // GMM resource descriptor
    uint32_t                frame_idx;
    void                   *pDecCtx;
    void                   *pVpCtx;

    uint32_t                            curCtxType;                // indicate current surface is using in which context type.
    DDI_MEDIA_STATUS_REPORT_QUERY_STATE curStatusReportQueryState; // indicate status report is queried or not.
    DDI_MEDIA_SURFACE_STATUS_REPORT     curStatusReport;           // union for both decode and vpp status.

    PDDI_MEDIA_CONTEXT      pMediaCtx; // Media driver Context
    PMEDIA_SEM_T            pCurrentFrameSemaphore;   // to sync render target for hybrid decoding multi-threading mode
    PMEDIA_SEM_T            pReferenceFrameSemaphore; // to sync reference frame surface. when this semaphore is posted, the surface is not used as reference frame, and safe to be destroied

    uint8_t                 *pSystemShadow;           // Shadow surface in system memory

    uint32_t                uiMapFlag;
} DDI_MEDIA_SURFACE, *PDDI_MEDIA_SURFACE;

typedef struct _DDI_MEDIA_BUFFER
{
    uint32_t               iSize;
    uint32_t               uiWidth;
    uint32_t               uiHeight;
    uint32_t               uiPitch;
    uint32_t               uiNumElements;
    uint32_t               uiOffset;
    // vaBuffer type
    uint32_t               uiType;
    DDI_MEDIA_FORMAT       format;
    uint32_t               uiLockedBufID;
    uint32_t               uiLockedImageID;
    int32_t                iRefCount;
    uint32_t               TileType;
    uint8_t               *pData;
    uint32_t               bMapped;
    MOS_LINUX_BO          *bo;
    uint32_t               name;
    uint32_t               uiMemtype;
    uint32_t               uiExportcount;
    uintptr_t              handle;

    bool                   bCFlushReq; // No LLC between CPU & GPU, requries to call CPU Flush for CPU mapped buffer
    PDDI_MEDIA_SURFACE     pSurface;
    GMM_RESOURCE_INFO     *pGmmResourceInfo; // GMM resource descriptor
    PDDI_MEDIA_CONTEXT     pMediaCtx; // Media driver Context
} DDI_MEDIA_BUFFER, *PDDI_MEDIA_BUFFER;

typedef struct _DDI_MEDIA_SURFACE_HEAP_ELEMENT
{
    PDDI_MEDIA_SURFACE                      pSurface;
    uint32_t                                uiVaSurfaceID;
    struct _DDI_MEDIA_SURFACE_HEAP_ELEMENT *pNextFree;
}DDI_MEDIA_SURFACE_HEAP_ELEMENT, *PDDI_MEDIA_SURFACE_HEAP_ELEMENT;

typedef struct _DDI_MEDIA_BUFFER_HEAP_ELEMENT
{
    PDDI_MEDIA_BUFFER                       pBuffer;
    void                                   *pCtx;
    uint32_t                                uiCtxType;
    uint32_t                                uiVaBufferID;
    struct _DDI_MEDIA_BUFFER_HEAP_ELEMENT  *pNextFree;
}DDI_MEDIA_BUFFER_HEAP_ELEMENT, *PDDI_MEDIA_BUFFER_HEAP_ELEMENT;

typedef struct _DDI_MEDIA_IMAGE_HEAP_ELEMENT
{
    VAImage                                *pImage;
    uint32_t                                uiVaImageID;
    struct _DDI_MEDIA_IMAGE_HEAP_ELEMENT   *pNextFree;
}DDI_MEDIA_IMAGE_HEAP_ELEMENT, *PDDI_MEDIA_IMAGE_HEAP_ELEMENT;

typedef struct _DDI_MEDIA_VACONTEXT_HEAP_ELEMENT
{
    void                                       *pVaContext;
    uint32_t                                    uiVaContextID;
    struct _DDI_MEDIA_VACONTEXT_HEAP_ELEMENT   *pNextFree;
}DDI_MEDIA_VACONTEXT_HEAP_ELEMENT, *PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT;

typedef struct _DDI_MEDIA_HEAP
{
    void               *pHeapBase;
    uint32_t            uiHeapElementSize;
    uint32_t            uiAllocatedHeapElements;
    void               *pFirstFreeHeapElement;
}DDI_MEDIA_HEAP, *PDDI_MEDIA_HEAP;

#ifndef ANDROID
typedef struct _DDI_X11_FUNC_TABLE
{
    HMODULE pX11LibHandle;    // handle to dlopen libX11

    // 5 functions from libX11 used by vpgPutSurface (Linux) so far
    void   *pfnDefaultVisual;
    void   *pfnXCreateGC;
    void   *pfnXFreeGC;
    void   *pfnXCreateImage;
    void   *pfnXDestroyImage;
    void   *pfnXPutImage;
}DDI_X11_FUNC_TABLE, *PDDI_X11_FUNC_TABLE;
#endif

//!
//! \struct DDI_MEDIA_CONTEXT
//! \brief  Media heap for shared internal structures
//!
struct DDI_MEDIA_CONTEXT
{
    MOS_BUFMGR         *pDrmBufMgr;

    // handle for /dev/dri/card0
    int32_t             fd;
    int32_t             iDeviceId;
    bool                bIsAtomSOC;

    MEDIA_FEATURE_TABLE SkuTable;
    MEDIA_WA_TABLE      WaTable;

    PDDI_MEDIA_HEAP     pSurfaceHeap;
    uint32_t            uiNumSurfaces;

    PDDI_MEDIA_HEAP     pBufferHeap;
    uint32_t            uiNumBufs;

    PDDI_MEDIA_HEAP     pImageHeap;
    uint32_t            uiNumImages;

    PDDI_MEDIA_HEAP     pDecoderCtxHeap;
    uint32_t            uiNumDecoders;

    PDDI_MEDIA_HEAP     pEncoderCtxHeap;
    uint32_t            uiNumEncoders;

    PDDI_MEDIA_HEAP     pVpCtxHeap;
    uint32_t            uiNumVPs;

    PDDI_MEDIA_HEAP     pCmCtxHeap;
    uint32_t            uiNumCMs;

    PDDI_MEDIA_HEAP     pMfeCtxHeap;
    uint32_t            uiNumMfes;

    // display info
    uint32_t            uiDisplayWidth;
    uint32_t            uiDisplayHeight;

    // media context reference number
    uint32_t            uiRef;

    // modulized Gpu context and cmd buffer
    bool                modularizedGpuCtxEnabled;
    OsContext          *m_osContext;
    GpuContextMgr      *m_gpuContextMgr;
    CmdBufMgr          *m_cmdBufMgr;

    // Apogeio MOS module
    MOS_DEVICE_HANDLE   m_osDeviceContext = MOS_INVALID_HANDLE;

    // mutexs to protect the shared resource among multiple context
    MEDIA_MUTEX_T       SurfaceMutex;
    MEDIA_MUTEX_T       BufferMutex;
    MEDIA_MUTEX_T       ImageMutex;
    MEDIA_MUTEX_T       DecoderMutex;
    MEDIA_MUTEX_T       EncoderMutex;
    MEDIA_MUTEX_T       VpMutex;
    MEDIA_MUTEX_T       CmMutex;
    MEDIA_MUTEX_T       MfeMutex;

    // GT system Info
    MEDIA_SYSTEM_INFO  *pGtSystemInfo;

    // Media memory decompression data structure
    void               *pMediaMemDecompState;

    // Media memory decompression function
    void (* pfnMemoryDecompress)(
        PMOS_CONTEXT  pMosCtx,
        PMOS_RESOURCE pOsResource);

    PLATFORM            platform;

    MediaLibvaCaps     *m_caps;

    GMM_CLIENT_CONTEXT  *pGmmClientContext;

    GmmExportEntries   GmmFuncs;

    // Aux Table Manager
    AuxTableMgr         *m_auxTableMgr;

    bool                m_useSwSwizzling;
    bool                m_tileYFlag;

#if !defined(ANDROID) && defined(X11_FOUND)
    // X11 Func table, for vpgPutSurface (Linux)
    PDDI_X11_FUNC_TABLE X11FuncTable;

    /* VA/DRI (X11) specific data */
    struct va_dri_output *dri_output;
    //vpgPutSurfaceLinuxHW acceleration hack
    MEDIA_MUTEX_T    PutSurfaceRenderMutex;
    MEDIA_MUTEX_T    PutSurfaceSwapBufferMutex;
#endif
};

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

#endif
