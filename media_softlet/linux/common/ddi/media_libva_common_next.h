/*
* Copyright (c) 2021-2022, Intel Corporation
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

//! \file     media_libva_common_next.h
//! \brief    libva common next head file
//!

#ifndef __MEDIA_LIBVA_COMMON_NEXT_H__
#define __MEDIA_LIBVA_COMMON_NEXT_H__

#include <va/va.h>
#include <va/va_backend.h>
#include <semaphore.h>
#include "GmmLib.h"
#include "mos_bufmgr_api.h"
#include "mos_defs_specific.h"

#define DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT       127
#define DDI_MEDIA_MAX_INSTANCE_NUMBER              0x0FFFFFFF

#define DDI_MEDIA_VACONTEXTID_OFFSET_DECODER       0x10000000
#define DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER       0x20000000
#define DDI_MEDIA_VACONTEXTID_OFFSET_PROT          0x30000000
#define DDI_MEDIA_VACONTEXTID_OFFSET_VP            0x40000000
#define DDI_MEDIA_VACONTEXTID_BASE                 0x90000000
#define DDI_MEDIA_MASK_VACONTEXT_TYPE              0xF0000000

#define DDI_MEDIA_SOFTLET_VACONTEXTID_DECODER_OFFSET      (DDI_MEDIA_VACONTEXTID_BASE + DDI_MEDIA_VACONTEXTID_OFFSET_DECODER)
#define DDI_MEDIA_SOFTLET_VACONTEXTID_ENCODER_OFFSET      (DDI_MEDIA_VACONTEXTID_BASE + DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER)
#define DDI_MEDIA_SOFTLET_VACONTEXTID_CP_OFFSET           (DDI_MEDIA_VACONTEXTID_BASE + DDI_MEDIA_VACONTEXTID_OFFSET_PROT)
#define DDI_MEDIA_SOFTLET_VACONTEXTID_VP_OFFSET           (DDI_MEDIA_VACONTEXTID_BASE + DDI_MEDIA_VACONTEXTID_OFFSET_VP)

#define DDI_MEDIA_MASK_VACONTEXTID                 0x0FFFFFFF

#define DDI_MEDIA_CONTEXT_TYPE_DECODER             1
#define DDI_MEDIA_CONTEXT_TYPE_ENCODER             2
#define DDI_MEDIA_CONTEXT_TYPE_VP                  3
#define DDI_MEDIA_CONTEXT_TYPE_MEDIA               4
#define DDI_MEDIA_CONTEXT_TYPE_CM                  5
#define DDI_MEDIA_CONTEXT_TYPE_PROTECTED           6
#define DDI_MEDIA_CONTEXT_TYPE_MFE                 7
#define DDI_MEDIA_CONTEXT_TYPE_NONE                0
#define DDI_MEDIA_INVALID_VACONTEXTID              0

#define DDI_MEDIA_MAX_COLOR_PLANES                 4       //Maximum color planes supported by media driver, like (A/R/G/B in different planes)

#define DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE   0       // Dec config_id starts at this value
#define DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_MAX    1023
#define DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE   1024    // Enc config_id starts at this value
#define DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_MAX    2047
#define DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE          2048    // VP config_id starts at this value
#define DDI_VP_GEN_CONFIG_ATTRIBUTES_MAX           4091
#define DDI_CP_GEN_CONFIG_ATTRIBUTES_BASE          4092    // CP config_id starts at this value

//codec specific defination
#define DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1          16
#define DDI_CODEC_GEN_MAX_SURFACE_ATTRIBUTES       40

#define DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB      3
#define DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB      3
#define DDI_CODEC_VDENC_MAX_L0_REF_FRAMES          3
#define DDI_CODEC_VDENC_MAX_L1_REF_FRAMES          0
#define DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_RAB_AVC  1

#define DDI_CODEC_FEI_MAX_NUM_MVPREDICTOR     4
typedef pthread_mutex_t  MEDIA_MUTEX_T, *PMEDIA_MUTEX_T;
#define MEDIA_MUTEX_INITIALIZER  PTHREAD_MUTEX_INITIALIZER
typedef sem_t            MEDIA_SEM_T, *PMEDIA_SEM_T;

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
    Media_Format_R10G10B10X2 ,
    Media_Format_B10G10R10X2 ,
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
#if VA_CHECK_VERSION(1, 13, 0)
    Media_Format_XYUV        ,
#endif
    Media_Format_Y410        ,
    Media_Format_Y416        ,
    Media_Format_Y8          ,
    Media_Format_Y16S        ,
    Media_Format_Y16U        ,
    Media_Format_VYUY        ,
    Media_Format_YVYU        ,
    Media_Format_A16R16G16B16,
    Media_Format_A16B16G16R16,
    Media_Format_P012        ,
#if VA_CHECK_VERSION(1, 9, 0)
    Media_Format_Y212        ,
    Media_Format_Y412        ,
#endif
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
    uint64_t   modifier;                              // used for VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2 or VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3
} DDI_MEDIA_SURFACE_DESCRIPTOR,*PDDI_MEDIA_SURFACE_DESCRIPTOR;

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
//!
//! \struct DDI_MEDIA_CONTEXT
//! \brief  Ddi media context
//!
struct DDI_MEDIA_CONTEXT;
typedef struct DDI_MEDIA_CONTEXT *PDDI_MEDIA_CONTEXT;

struct _DDI_MEDIA_BUFFER;
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
    _DDI_MEDIA_BUFFER       *pShadowBuffer;

    uint32_t                uiMapFlag;

    uint32_t                uiVariantFlag;
    int                     memType;
} DDI_MEDIA_SURFACE, *PDDI_MEDIA_SURFACE;

typedef struct _DDI_MEDIA_BUFFER
{
    uint32_t               iSize             = 0;
    uint32_t               uiWidth           = 0;
    uint32_t               uiHeight          = 0;
    uint32_t               uiPitch           = 0;
    uint32_t               uiNumElements     = 0;
    uint32_t               uiOffset          = 0;
    // vaBuffer type
    uint32_t               uiType            = 0;
    DDI_MEDIA_FORMAT       format            = Media_Format_Count;
    uint32_t               uiLockedBufID     = 0;
    uint32_t               uiLockedImageID   = 0;
    int32_t                iRefCount         = 0;
    uint32_t               TileType          = 0;
    uint8_t               *pData             = nullptr;
    uint32_t               bMapped           = 0;
    MOS_LINUX_BO          *bo                = nullptr;
    uint32_t               name              = 0;
    uint32_t               uiMemtype         = 0;
    uint32_t               uiExportcount     = 0;
    uintptr_t              handle            = 0;
    bool                   bPostponedBufFree = false;

    bool                   bCFlushReq        = false; // No LLC between CPU & GPU, requries to call CPU Flush for CPU mapped buffer
    bool                   bUseSysGfxMem     = false;
    PDDI_MEDIA_SURFACE     pSurface          = nullptr;
    GMM_RESOURCE_INFO     *pGmmResourceInfo  = nullptr; // GMM resource descriptor
    PDDI_MEDIA_CONTEXT     pMediaCtx         = nullptr; // Media driver Context
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
    uint32_t           uiHeapElementSize;
    uint32_t           uiAllocatedHeapElements;
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

#ifdef MEDIA_SOFTLET

#include "media_interfaces_hwinfo.h"
class MediaLibvaCapsNext;
class OsContext;
class GpuContextMgr;
#include "ddi_media_functions.h"
#include "ddi_media_context.h"

typedef struct DDI_MEDIA_CONTEXT *PDDI_MEDIA_CONTEXT;

#else // MEDIA_SOFTLET

#include "media_libva_common.h"

#endif // MEDIA_SOFTLET

static __inline PDDI_MEDIA_CONTEXT GetMediaContext (VADriverContextP ctx)
{
    return (PDDI_MEDIA_CONTEXT)ctx->pDriverData;
}

#ifndef CONTEXT_PRIORITY_MAX
#define CONTEXT_PRIORITY_MAX 1024
#endif

class MediaLibvaCommonNext
{
public:
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
    static DDI_MEDIA_SURFACE *GetSurfaceFromVASurfaceID(PDDI_MEDIA_CONTEXT mediaCtx, VASurfaceID surfaceID);

    //!
    //! \brief  Get VA surface ID  from surface
    //!
    //! \param  [in] surface
    //!     surface
    //!
    //! \return VASurfaceID
    //!     VA Surface ID
    //!
    static VASurfaceID GetVASurfaceIDFromSurface(PDDI_MEDIA_SURFACE surface);

    //!
    //! \brief  Replace the surface with given format
    //!
    //! \param  [in] surface
    //!     Pointer to the old surface
    //! \param  [in] expectedFormat
    //!     VA surface ID
    //!
    //! \return DDI_MEDIA_SURFACE*
    //!     Pointer to new ddi media surface
    //!
    static PDDI_MEDIA_SURFACE ReplaceSurfaceWithNewFormat(PDDI_MEDIA_SURFACE surface, DDI_MEDIA_FORMAT expectedFormat);

    //!
    //! \brief  replace the surface with correlation variant format
    //!
    //! \param  [in] surface
    //!     Pointer to the old surface
    //!
    //! \return DDI_MEDIA_SURFACE*
    //!     Pointer to new ddi media surface
    //!
    static PDDI_MEDIA_SURFACE ReplaceSurfaceWithVariant(PDDI_MEDIA_SURFACE surface, VAEntrypoint entrypoint);

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
    static DDI_MEDIA_BUFFER* GetBufferFromVABufferID(
        PDDI_MEDIA_CONTEXT mediaCtx,
        VABufferID         bufferID);

    //!
    //! \brief  Media buffer to mos resource
    //!
    //! \param  [in] mediaBuffer
    //!     Ddi media buffer
    //! \param  [in] mhalOsResource
    //!     Mos resource
    //!
    static void MediaBufferToMosResource(DDI_MEDIA_BUFFER *mediaBuffer, MOS_RESOURCE *mhalOsResource);

    //!
    //! \brief  Media surface to mos resource
    //!
    //! \param  [in] mediaSurface
    //!     Ddi media surface
    //! \param  [in] mhalOsResource
    //!     Mos resource
    //!
    static void MediaSurfaceToMosResource(DDI_MEDIA_SURFACE *mediaSurface, MOS_RESOURCE *mhalOsResource);

    //!
    //! \brief  Get PVA context from heap
    //!
    //! \param  [in] mediaHeap
    //!         Pointer to ddi media heap
    //! \param  [in] index
    //!         the index
    //! \param  [in] mutex
    //!         the mutex
    //!
    static void* GetVaContextFromHeap(PDDI_MEDIA_HEAP mediaHeap, uint32_t index, PMOS_MUTEX mutex);

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
    static void* GetContextFromContextID(VADriverContextP ctx, VAContextID vaCtxID, uint32_t *ctxType);

    //!
    //! \brief  Get ctx type from VA buffer ID
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context
    //! \param  [in] bufferID
    //!         VA buffer ID
    //!
    //! \return uint32_t
    //1     Context type
    //!
    static uint32_t GetCtxTypeFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID);

    //!
    //! \brief  Get ctx from VA buffer ID
    //!
    //! \param  [in] mediaCtx
    //!         pddi media context
    //! \param  [in] bufferID
    //!         VA Buffer ID
    //!
    //! \return void*
    //!     Pointer to buffer heap element context
    //!
    static void* GetCtxFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID);

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
    static int32_t GetGpuPriority(VADriverContextP ctx, VABufferID *buffers, int32_t numBuffers, bool *updatePriority, int32_t *priority);

    //!
    //! \brief  Move the priority bufferID to the end of buffers
    //!
    //! \param  [in] buffers
    //!     VA buffer ID
    //! \param  [in] priorityIndexInBuf
    //!     the priority VA buffer ID
    //! \param  [in] numBuffers
    //!     Number of buffers
    //!
    static void MovePriorityBufferIdToEnd (VABufferID *buffers, int32_t priorityIndexInBuf, int32_t numBuffers);
MEDIA_CLASS_DEFINE_END(MediaLibvaCommonNext)
};

#endif //__MEDIA_LIBVA_COMMON_NEXT_H__
