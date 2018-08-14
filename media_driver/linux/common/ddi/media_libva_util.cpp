/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file      media_libva_util.cpp
//! \brief     libva(and its extension) utility
//!
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>

#include "media_libva_util.h"
#include "mos_utilities.h"
#include "mos_os.h"
#include "hwinfo_linux.h"
#include "media_ddi_decode_base.h"
#include "media_ddi_encode_base.h"
#include "media_libva_decoder.h"
#include "media_libva_encoder.h"
#include "media_libva_caps.h"

#ifdef DEBUG
static int32_t         frameCountFps   = -1;
static struct timeval  tv1;
static pthread_mutex_t fpsMutex        = PTHREAD_MUTEX_INITIALIZER;
static int32_t         vaFpsSampleSize = 100;

#define LENGTH_OF_FPS_FILE_NAME 128

#ifdef ANDROID
#define FPS_FILE_NAME   "/mnt/sdcard/fps.txt"
#else
#define FPS_FILE_NAME   "./fps.txt"
#endif
#endif
#ifdef DEBUG
void DdiMediaUtil_MediaPrintFps()
{
    struct timeval tv2;

    if (0 == vaFpsSampleSize) 
    {
        return;
    }
    gettimeofday(&tv2, 0);

    pthread_mutex_lock(&fpsMutex);
    if (-1 == frameCountFps) 
    {
        gettimeofday(&tv1, 0);
    }

    if (++frameCountFps >= vaFpsSampleSize) 
    {
        char   fpsFileName[LENGTH_OF_FPS_FILE_NAME];
        FILE   *fp = nullptr;
        char   temp[LENGTH_OF_FPS_FILE_NAME];

        int64_t diff  = (tv2.tv_sec - tv1.tv_sec)*1000000 + tv2.tv_usec - tv1.tv_usec;
        float fps     = frameCountFps / (diff / 1000000.0);
        DDI_NORMALMESSAGE("FPS:%6.4f, Interval:%11lu.", fps,((uint64_t)tv2.tv_sec)*1000 + (tv2.tv_usec/1000));
        sprintf(temp,"FPS:%6.4f, Interval:%11lu\n", fps,((uint64_t)tv2.tv_sec)*1000 + (tv2.tv_usec/1000));

        MOS_ZeroMemory(fpsFileName,LENGTH_OF_FPS_FILE_NAME);
        sprintf(fpsFileName, FPS_FILE_NAME);
        if ((fp = fopen(fpsFileName, "wb")) == nullptr) 
        {
            pthread_mutex_unlock(&fpsMutex);
            DDI_ASSERTMESSAGE("Unable to open fps file.");
        }

        fwrite(temp, 1, strlen(temp), fp);
        fclose(fp);
        frameCountFps = -1;
    }
    pthread_mutex_unlock(&fpsMutex);
}
#else
void DdiMediaUtil_MediaPrintFps()
{
    return;
}
#endif

#ifdef ANDROID
#define GTT_SIZE_THRESHOLD  (4096*4096*3)    //use the maximum 4K resolution YUV 444 as the threshold
// The following implemention refers to Solo_PerformManualSwizzling in mos_os_solo.c.
static __inline int32_t SwizzleOffset(
    // Linear Offset:
    int32_t             OffsetX,        // Horizontal byte offset from left edge of tiled surface.
    int32_t             OffsetY,        // Vertical offset from top of tiled surface.
    // Swizzling Parameters:
    int32_t             Pitch,          // Row-to-row byte stride.
    uint32_t            TileFormat,     // Either 'x' or 'y'--for X-Major or Y-Major tiling, respectively.
    int32_t             CsxSwizzle)     // (boolean) Additionally perform Channel Select XOR swizzling.
{
    int32_t LBits, LPos; // Size and swizzled position of the Line component.
    if (TileFormat == I915_TILING_NONE)
    {
        return(OffsetY * Pitch + OffsetX);
    }

    if (TileFormat == I915_TILING_Y)
    {
        LBits = 5; // Log2(TileY.Height = 32)
        LPos = 4;  // Log2(TileY.PseudoWidth = 16)
    }
    else //if (TileFormat == I915_TILING_X)
    {
        LBits = 3; // Log2(TileX.Height = 8)
        LPos = 9;  // Log2(TileX.Width = 512)
    }

    int32_t row  =  OffsetY >> LBits;               // OffsetY / LinesPerTile
    int32_t line =  OffsetY & ((1 << LBits) - 1);   // OffsetY % LinesPerTile
    int32_t col  =  OffsetX >> LPos;                // OffsetX / BytesPerLine
    int32_t x    =  OffsetX & ((1 << LPos) - 1);    // OffsetX % BytesPerLine

    int32_t swizzledOffset =
        (((((row * (Pitch >> LPos)) + col) << LBits) + line) << LPos) + x;
    //                V                V                 V
    //                / BytesPerLine   * LinesPerTile    * BytesPerLine

    /// Channel Select XOR Swizzling ///////////////////////////////////////////
    if (CsxSwizzle)
    {
        if (TileFormat == I915_TILING_Y) // A6 = A6 ^ A9
        {
            swizzledOffset ^= ((swizzledOffset >> (9 - 6)) & 0x40);
        }
        else //if (TileFormat == I915_TILING_X) // A6 = A6 ^ A9 ^ A10
        {
            swizzledOffset ^= (((swizzledOffset >> (9 - 6)) ^ (swizzledOffset >> (10 - 6))) & 0x40);
        }
    }

    return(swizzledOffset);
}

static void SwizzleData(
    uint8_t  *src,
    uint8_t  *dst,
    uint32_t  srcTiling,
    uint32_t  dstTiling,
    int32_t   height,
    int32_t   pitch)
{
#define IS_TILED(_a)                ((_a) != I915_TILING_NONE)
#define IS_TILED_TO_LINEAR(_a, _b)  (IS_TILED(_a) && !IS_TILED(_b))
#define IS_LINEAR_TO_TILED(_a, _b)  (!IS_TILED(_a) && IS_TILED(_b))

    int32_t y,linearOffset,tileOffset;
    // Translate from one format to another
    for (y = 0, linearOffset = 0, tileOffset = 0; y < height; y++)
    {
        for (int32_t x = 0; x < pitch; x++, linearOffset++)
        {
            // x or y --> linear
            if (IS_TILED_TO_LINEAR(srcTiling, dstTiling))
            {
                tileOffset = SwizzleOffset(
                    x,
                    y,
                    pitch,
                    srcTiling,
                    false);

                *(dst + linearOffset) = *(src + tileOffset);
            }
            // linear --> x or y
            else if (IS_LINEAR_TO_TILED(srcTiling, dstTiling))
            {
                tileOffset = SwizzleOffset(
                    x,
                    y,
                    pitch,
                    dstTiling,
                    false);

                *(dst + tileOffset) = *(src + linearOffset);
            }
            else
            {
                MOS_OS_ASSERT(0);
            }
        }
    }
}

static bool NeedSwizzleData(PDDI_MEDIA_SURFACE surface, bool lock)
{
    DDI_CHK_NULL(surface, "nullptr surface", false);
    DDI_CHK_NULL(surface->pGmmResourceInfo, "nullptr pGmmResourceInfo", false);
    uint32_t pitch = (uint32_t)surface->pGmmResourceInfo->GetRenderPitch();
    uint32_t size  = (uint32_t)surface->pGmmResourceInfo->GetSizeSurface();
    GMM_RESOURCE_FLAG gmmFlags = surface->pGmmResourceInfo->GetResFlags();

    if (gmmFlags.Gpu.RenderTarget      &&
        gmmFlags.Gpu.UnifiedAuxSurface &&
        gmmFlags.Gpu.CCS)
    {
        size = size - (uint32_t)(surface->pGmmResourceInfo->GetSizeAuxSurface(GMM_AUX_SURF));
    }

    uint8_t *resourceBase = (uint8_t*)MOS_AllocAndZeroMemory(size);
    DDI_CHK_NULL(resourceBase, "nullptr resourceBase", false);

    if (lock)
    {
        SwizzleData((uint8_t*) surface->bo->virt, resourceBase, surface->TileType, I915_TILING_NONE, size / pitch, pitch);
    }
    else
    {
        SwizzleData((uint8_t*) surface->bo->virt, resourceBase, I915_TILING_NONE, surface->TileType, size / pitch, pitch);
    }
    MOS_SecureMemcpy((uint8_t*) surface->bo->virt, size, resourceBase, size);
    MOS_FreeMemory(resourceBase);

    return true;
}
#endif
/*
 * DdiMediaUtil_IsExternalSurface
 *    Descripion: if the bo of media surface was allocated from App,
 *                should return true, otherwise, false. In current implemeation
 *                external buffer passed with pSurfDesc.
*/
bool DdiMediaUtil_IsExternalSurface(PDDI_MEDIA_SURFACE surface)
{
    if ( nullptr == surface )
    {
        return false;
    }
    else if ( surface->pSurfDesc == nullptr )
    {
        return false;
    }

    return true;
}

//!
//! \brief  Allocate surface
//! 
//! \param  [in] format
//!         Ddi media format
//! \param  [in] width
//!         Width of the region
//! \param  [in] height
//!         Height of the region
//! \param  [out] mediaSurface
//!         Pointer to ddi media surface
//! \param  [in] mediaDrvCtx
//!         Pointer to ddi media context
//!         
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMediaUtil_AllocateSurface(
    DDI_MEDIA_FORMAT            format,
    int32_t                     width,
    int32_t                     height,
    PDDI_MEDIA_SURFACE          mediaSurface,
    PDDI_MEDIA_CONTEXT          mediaDrvCtx)
{
    int32_t                     pitch = 0;
    MOS_LINUX_BO               *bo = nullptr;
    GMM_RESCREATE_PARAMS        gmmParams;
    GMM_RESOURCE_INFO          *gmmResourceInfo;
    bool                        grallocAllocation;

    DDI_CHK_NULL(mediaSurface, "mediaSurface is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaDrvCtx, "mediaDrvCtx is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaDrvCtx->pGmmClientContext, "mediaDrvCtx->pGmmClientContext is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);

    int32_t size          = 0;
    uint32_t tileformat   = I915_TILING_NONE;
    VAStatus hRes         = VA_STATUS_SUCCESS;
    int32_t alignedHeight = height;

    switch (format)
    {
        case Media_Format_X8R8G8B8:
        case Media_Format_X8B8G8R8:
        case Media_Format_A8B8G8R8:
        case Media_Format_R8G8B8A8:
        case Media_Format_R5G6B5:
        case Media_Format_R8G8B8:
        case Media_Format_R10G10B10A2:
        case Media_Format_B10G10R10A2: 
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != mediaSurface->surfaceUsageHint)
            {
                 tileformat = I915_TILING_NONE;
                 break;
            }
        case Media_Format_NV21:
        case Media_Format_YV12:
        case Media_Format_I420:
        case Media_Format_IYUV:
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != mediaSurface->surfaceUsageHint)
            {
                 tileformat = I915_TILING_NONE;
                 break;
            }
        case Media_Format_RGBP:
        case Media_Format_UYVY:
        case Media_Format_A8R8G8B8:
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != mediaSurface->surfaceUsageHint &&
                !(mediaSurface->surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_DECODER))
            {
                 tileformat = I915_TILING_NONE;
                 break;
            }
        case Media_Format_NV12:
        case Media_Format_444P:
        case Media_Format_422H:
        case Media_Format_411P:
        case Media_Format_422V:
        case Media_Format_IMC3:
        case Media_Format_400P:
        case Media_Format_P010:
        case Media_Format_P016:
        case Media_Format_YUY2:
        case Media_Format_Y210:
        case Media_Format_Y216:
        case Media_Format_AYUV:
        case Media_Format_Y410:
        case Media_Format_Y416:     
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != mediaSurface->surfaceUsageHint)
            {
#if UFO_GRALLOC_NEW_FORMAT
                 //Planar type surface align 64 to improve performance.
                alignedHeight = MOS_ALIGN_CEIL(height, 64);
#else
                //Planar type surface align 32 to improve performance.
                alignedHeight = MOS_ALIGN_CEIL(height, 32);
#endif
            }
            tileformat  = I915_TILING_Y;
            break;
        case Media_Format_Buffer:
            tileformat = I915_TILING_NONE;
            break;
        default:
            DDI_ASSERTMESSAGE("Unsupported format");
            hRes = VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
            goto finish;
    }

    if( DdiMediaUtil_IsExternalSurface(mediaSurface) )
    { 
        // DRM buffer allocated by Application, No need to re-allocate new DRM buffer
         if( (mediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM)
             || (mediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME)
           )
        {
            if (mediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM)
            {
                bo = mos_bo_gem_create_from_name(mediaDrvCtx->pDrmBufMgr, "MEDIA", mediaSurface->pSurfDesc->ulBuffer);
            }
            else
            {
                bo = mos_bo_gem_create_from_prime(mediaDrvCtx->pDrmBufMgr, mediaSurface->pSurfDesc->ulBuffer, mediaSurface->pSurfDesc->uiSize);
            }

            if( bo != nullptr )
            {
                uint32_t swizzle_mode;

                pitch = mediaSurface->pSurfDesc->uiPitches[0];

                //Overwirte the tileformat matches with the right buffer
                mos_bo_get_tiling(bo, &tileformat, &swizzle_mode);
            }
            else
            {
                DDI_ASSERTMESSAGE("Failed to create drm buffer object according to input buffer descriptor.");
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
        }
        else if( mediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR )
        {

            pitch    = mediaSurface->pSurfDesc->uiPitches[0];

#ifdef ANDROID
#ifdef DRM_IOCTL_I915_GEM_USERPTR
            bo = mos_bo_alloc_userptr( mediaDrvCtx->pDrmBufMgr,
                                          "SysSurface",
                                          (void *)mediaSurface->pSurfDesc->ulBuffer,
                                          mediaSurface->pSurfDesc->uiTile,
                                          pitch,
                                          mediaSurface->pSurfDesc->uiBuffserSize,
                                          I915_USERPTR_UNSYNCHRONIZED
                                         );
#else
            bo = mos_bo_alloc_vmap( mediaDrvCtx->pDrmBufMgr,
                                          "SysSurface",
                                          (void *)mediaSurface->pSurfDesc->ulBuffer,
                                          mediaSurface->pSurfDesc->uiTile,
                                          pitch,
                                          mediaSurface->pSurfDesc->uiBuffserSize,
                                          0
                                         );
#endif
#else
            bo = nullptr;
#endif
            if( bo != nullptr )
            {
                uint32_t swizzle_mode;

                //Overwrite the tile format that matches  the exteral buffer
                mos_bo_get_tiling(bo, &tileformat, &swizzle_mode);
                DDI_VERBOSEMESSAGE("Success to create drm buffer vmap.");
           }
           else
           {
               DDI_ASSERTMESSAGE("Failed to create drm buffer vmap.");
               return VA_STATUS_ERROR_ALLOCATION_FAILED;
           }
        }
        else
        {
            DDI_ASSERTMESSAGE("Input buffer descriptor (%d) is not supported by current driver.", mediaSurface->pSurfDesc->uiFlags);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }

    // Create GmmResourceInfo
    MOS_ZeroMemory(&gmmParams, sizeof(gmmParams));
    if (DdiMediaUtil_IsExternalSurface(mediaSurface))
    {
        gmmParams.BaseWidth         = mediaSurface->iWidth;
        gmmParams.BaseHeight        = mediaSurface->iHeight;
    }
    else
    {
        gmmParams.BaseWidth         = width;
        gmmParams.BaseHeight        = alignedHeight;
    }
    
    gmmParams.ArraySize             = 1;
    gmmParams.Type                  = RESOURCE_2D;
    //gmmParams.Format                = DdiMediaUtil_ConvertMediaFmtToGmmFmt(format);
    gmmParams.Format                = mediaDrvCtx->m_caps->ConvertMediaFmtToGmmFmt(format);
    
    DDI_CHK_CONDITION(gmmParams.Format == GMM_FORMAT_INVALID, 
                         "Unsupported format", 
                         VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT);
    switch (tileformat)
    {
        case I915_TILING_Y:
            gmmParams.Flags.Info.TiledY    = true;
            // Disable MMC for application required surfaces, because some cases' output streams have corruption.
            gmmParams.Flags.Gpu.MMC    = false;
            if ( mediaDrvCtx->m_auxTableMgr )
            {
                gmmParams.Flags.Gpu.MMC = true;
                gmmParams.Flags.Info.MediaCompressed = 1;
                gmmParams.Flags.Gpu.CCS = 1;
                gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;
                gmmParams.Flags.Gpu.RenderTarget = 1;
            }
            break;
        case I915_TILING_X:
            gmmParams.Flags.Info.TiledX    = true;
            break;
        default:
            gmmParams.Flags.Info.Linear    = true;
    }
       
    gmmParams.Flags.Gpu.Video = true;

    mediaSurface->pGmmResourceInfo = gmmResourceInfo = mediaDrvCtx->pGmmClientContext->CreateResInfoObject(&gmmParams);

    if(nullptr == gmmResourceInfo)
    {
        DDI_ASSERTMESSAGE("Gmm Create Resource Failed.");
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    uint32_t    gmmPitch;
    uint32_t    gmmSize;
    uint32_t    gmmHeight;
    gmmPitch    = (uint32_t)gmmResourceInfo->GetRenderPitch();
    gmmSize     = (uint32_t)gmmResourceInfo->GetSizeSurface();
    gmmHeight   = gmmResourceInfo->GetBaseHeight();

    if ( 0 == gmmPitch || 0 == gmmSize || 0 == gmmHeight)
    {
        DDI_ASSERTMESSAGE("Gmm Create Resource Failed.");
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    if (!DdiMediaUtil_IsExternalSurface(mediaSurface))
    {
        unsigned long  ulPitch;
#if defined(I915_PARAM_CREATE_VERSION)
        int32_t value = 0;
        int32_t ret = -1;
        drm_i915_getparam_t gp;
        memset( &gp, 0, sizeof(gp) );
        gp.value = &value;
        gp.param = I915_PARAM_CREATE_VERSION;
        ret = drmIoctl(mediaDrvCtx->fd, DRM_IOCTL_I915_GETPARAM, &gp);
        if ((0 == ret) && (tileformat != I915_TILING_NONE))
        {
            bo = mos_bo_alloc_tiled(mediaDrvCtx->pDrmBufMgr, "MEDIA", gmmPitch, gmmSize/gmmPitch, 1, &tileformat, (unsigned long *)&ulPitch, BO_ALLOC_STOLEN);
            if (nullptr == bo)
            {
                bo = mos_bo_alloc_tiled(mediaDrvCtx->pDrmBufMgr, "MEDIA", gmmPitch, gmmSize/gmmPitch, 1, &tileformat, (unsigned long *)&ulPitch, 0);
            }
            else
            {
                DDI_VERBOSEMESSAGE("Stolen memory is created sucessfully on AllocateSurface");
            }
            pitch = (int32_t)ulPitch;
        }
        else
#endif  
        {
            if ( tileformat == I915_TILING_NONE )
            {
                bo = mos_bo_alloc(mediaDrvCtx->pDrmBufMgr, "MEDIA", gmmSize, 4096);
                pitch = gmmPitch;
            }
            else
            {
                bo = mos_bo_alloc_tiled(mediaDrvCtx->pDrmBufMgr, "MEDIA", gmmPitch, gmmSize/gmmPitch, 1, &tileformat, (unsigned long *)&ulPitch, 0);
                pitch = (int32_t)ulPitch;
            }
        }
    }
    else
    {
        // Check Pitch and Size
#ifdef ANDROID
        if (gmmPitch > pitch)
#else
        if (gmmPitch > pitch || gmmSize > bo->size)
#endif
        {
            DDI_ASSERTMESSAGE("External Surface doesn't meet the reqirements of Media driver.");
            DdiMediaUtil_FreeSurface(mediaSurface);
            hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;

            goto finish;
        }
        
        // Check Alignment
         __GMM_BUFFER_TYPE   gmmRestrictions = {0};
        gmmResourceInfo->GetRestrictions(gmmRestrictions);
        unsigned long pitchAlign  = gmmRestrictions.RenderPitchAlignment;
        unsigned long sizeAlign   = gmmRestrictions.Alignment;
    
        if (!MOS_IS_ALIGNED(pitch, pitchAlign))
        {
            DDI_ASSERTMESSAGE("External Surface doesn't meet the reqirements of Media driver.");
            DdiMediaUtil_FreeSurface(mediaSurface);
            hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;

            goto finish;
        }
    }

    mediaSurface->bMapped = false;
    if (bo)
    {
        mediaSurface->format      = format;
        mediaSurface->iWidth      = width;
        mediaSurface->iHeight     = gmmHeight;
        mediaSurface->iRealHeight = height;
        mediaSurface->iPitch      = pitch;
        mediaSurface->iRefCount   = 0;
        mediaSurface->bo          = bo;
        mediaSurface->TileType    = tileformat;
        mediaSurface->isTiled     = (tileformat != I915_TILING_NONE) ? 1 : 0;
        mediaSurface->pData       = (uint8_t*) bo->virt;
        DDI_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource).",gmmSize, width, height);
    }
    else
    {
        DDI_ASSERTMESSAGE("Fail to Alloc %7d bytes (%d x %d resource).",gmmSize, width, height);
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

finish:
    return hRes;
}

//!
//! \brief  Allocate buffer
//! 
//! \param  [in] format
//!         Ddi media format
//! \param  [in] size
//!         Size of the region
//! \param  [out] mediaBuffer
//!         Pointer to ddi media buffer
//! \param  [in] bufmgr
//!         Mos buffer manager
//!         
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMediaUtil_AllocateBuffer(
    DDI_MEDIA_FORMAT            format,
    int32_t                     size,
    PDDI_MEDIA_BUFFER           mediaBuffer,
    MOS_BUFMGR                 *bufmgr)
{

    DDI_CHK_NULL(mediaBuffer, "mediaBuffer is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaBuffer->pMediaCtx, "mediaBuffer->pMediaCtx is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaBuffer->pMediaCtx->pGmmClientContext, "mediaBuffer->pMediaCtx->pGmmClientContext is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    if(format >= Media_Format_Count)
       return VA_STATUS_ERROR_INVALID_PARAMETER;

    VAStatus     hRes = VA_STATUS_SUCCESS;
    MOS_LINUX_BO *bo  = mos_bo_alloc(bufmgr, "Media Buffer", size, 4096);

    mediaBuffer->bMapped = false;
    if (bo)
    {
        mediaBuffer->format     = format;
        mediaBuffer->iSize      = size;
        mediaBuffer->iRefCount  = 0;
        mediaBuffer->bo         = bo;
        mediaBuffer->pData      = (uint8_t*) bo->virt;

        DDI_VERBOSEMESSAGE("Alloc %7d bytes resource.",size);
    }
    else
    {
        DDI_ASSERTMESSAGE("Fail to Alloc %7d bytes resource.",size);
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    // create fake GmmResourceInfo
    GMM_RESCREATE_PARAMS    gmmParams;
    MOS_ZeroMemory(&gmmParams, sizeof(gmmParams));
    gmmParams.BaseWidth             = 1;
    gmmParams.BaseHeight            = 1;
    gmmParams.ArraySize             = 0;
    gmmParams.Type                  = RESOURCE_1D;
    gmmParams.Format                = GMM_FORMAT_GENERIC_8BIT;
    gmmParams.Flags.Gpu.Video       = true;
    gmmParams.Flags.Info.Linear     = true;

    mediaBuffer->pGmmResourceInfo = mediaBuffer->pMediaCtx->pGmmClientContext->CreateResInfoObject(&gmmParams);

    DDI_CHK_NULL(mediaBuffer->pGmmResourceInfo, "pGmmResourceInfo is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    mediaBuffer->pGmmResourceInfo->OverrideSize(mediaBuffer->iSize);
    mediaBuffer->pGmmResourceInfo->OverrideBaseWidth(mediaBuffer->iSize);
    mediaBuffer->pGmmResourceInfo->OverridePitch(mediaBuffer->iSize);
finish:
    return hRes;
}

//!
//! \brief  Allocate 2D buffer
//! 
//! \param  [in] height
//!         Height of the region
//! \param  [in] width
//!         Width of the region
//! \param  [out] mediaBuffer
//!         Pointer to ddi media buffer
//! \param  [in] bufmgr
//!         Mos buffer manager
//!         
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMediaUtil_Allocate2DBuffer(
    int32_t                     height,
    int32_t                     width,
    PDDI_MEDIA_BUFFER           mediaBuffer,
    MOS_BUFMGR                 *bufmgr)
{
    DDI_CHK_NULL(mediaBuffer, "mediaBuffer is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaBuffer->pMediaCtx, "mediaBuffer->pMediaCtx is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaBuffer->pMediaCtx->pGmmClientContext, "mediaBuffer->pMediaCtx->pGmmClientContext is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);

    int32_t  size           = 0;
    uint32_t tileformat     = I915_TILING_NONE;
    VAStatus hRes           = VA_STATUS_SUCCESS;
     
    // Create GmmResourceInfo
    GMM_RESCREATE_PARAMS        gmmParams;
    MOS_ZeroMemory(&gmmParams, sizeof(gmmParams));
    gmmParams.BaseWidth             = width;
    gmmParams.BaseHeight            = height;
    gmmParams.ArraySize             = 1;
    gmmParams.Type                  = RESOURCE_2D;
    gmmParams.Format                = GMM_FORMAT_GENERIC_8BIT;

    DDI_CHK_CONDITION(gmmParams.Format == GMM_FORMAT_INVALID, 
                         "Unsupported format", 
                         VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT);

    gmmParams.Flags.Info.Linear = true;
    gmmParams.Flags.Gpu.Video   = true;
    GMM_RESOURCE_INFO          *gmmResourceInfo;
    mediaBuffer->pGmmResourceInfo = gmmResourceInfo = mediaBuffer->pMediaCtx->pGmmClientContext->CreateResInfoObject(&gmmParams);

    if(nullptr == gmmResourceInfo)
    {
        DDI_VERBOSEMESSAGE("Gmm Create Resource Failed.");
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    uint32_t    gmmPitch;
    uint32_t    gmmSize;
    uint32_t    gmmHeight;
    gmmPitch    = (uint32_t)gmmResourceInfo->GetRenderPitch();
    gmmSize     = (uint32_t)gmmResourceInfo->GetSizeSurface();
    gmmHeight   = gmmResourceInfo->GetBaseHeight();

    MOS_LINUX_BO  *bo;
    bo = mos_bo_alloc(bufmgr, "Media 2D Buffer", gmmSize, 4096); 

    mediaBuffer->bMapped = false;
    if (bo)
    {
        mediaBuffer->format     = Media_Format_2DBuffer;
        mediaBuffer->iWidth     = width;
        mediaBuffer->iHeight    = gmmHeight;
        mediaBuffer->iPitch     = gmmPitch;
        mediaBuffer->iSize      = gmmSize;
        mediaBuffer->iRefCount  = 0;
        mediaBuffer->bo         = bo;
        mediaBuffer->TileType   = tileformat;
        mediaBuffer->pData      = (uint8_t*) bo->virt; 
        DDI_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource)\n",size, width, height);
    }
    else
    {
        DDI_VERBOSEMESSAGE("Fail to Alloc %7d bytes (%d x %d resource)\n", size, width, height);
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

finish:
    return hRes;
}

VAStatus DdiMediaUtil_CreateSurface(DDI_MEDIA_SURFACE  *surface, PDDI_MEDIA_CONTEXT mediaDrvCtx)
{
    VAStatus hr = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_BUFFER);

    // better to differentiate 1D and 2D type
    hr = DdiMediaUtil_AllocateSurface(surface->format, 
                         surface->iWidth, 
                         surface->iHeight, 
                         surface, 
                         mediaDrvCtx);
    if (VA_STATUS_SUCCESS == hr && nullptr != surface->bo)
        surface->base = surface->name;

    return hr;
}

VAStatus DdiMediaUtil_CreateBuffer(DDI_MEDIA_BUFFER *buffer, MOS_BUFMGR *bufmgr)
{
    VAStatus hr = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(buffer, "nullptr buffer", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_CHK_LESS(buffer->format, Media_Format_Count, "Invalid buffer->format", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (buffer->format == Media_Format_CPU)
    {
        buffer->pData= (uint8_t*)MOS_AllocAndZeroMemory(buffer->iSize);
        if (nullptr == buffer->pData)
            hr = VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    else
    {
        if (Media_Format_2DBuffer == buffer->format)
        {
            hr = DdiMediaUtil_Allocate2DBuffer(buffer->iHeight,
                                  buffer->iWidth,
                                  buffer,
                                  bufmgr);
         }
         else
         {
             hr = DdiMediaUtil_AllocateBuffer(buffer->format, 
                                 buffer->iSize,
                                 buffer,
                                 bufmgr);
         }
    }

    buffer->uiLockedBufID   = VA_INVALID_ID;
    buffer->uiLockedImageID = VA_INVALID_ID;
    buffer->iRefCount       = 0;

    return hr;
}

// add thread protection for multiple thread?
void* DdiMediaUtil_LockSurface(DDI_MEDIA_SURFACE  *surface, uint32_t flag)
{
    DDI_CHK_NULL(surface, "nullptr surface", nullptr);
    DDI_CHK_NULL(surface->bo, "nullptr surface->bo", nullptr);
    if((false == surface->bMapped) && (0 == surface->iRefCount))
    {
        if (surface->pMediaCtx->bIsAtomSOC)
        {
#ifdef ANDROID
            if (surface->iWidth * surface->iHeight * 3 <= GTT_SIZE_THRESHOLD)
            {
                mos_gem_bo_map_gtt(surface->bo);
            }
            else
            {
                mos_bo_map(surface->bo, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));
                if (surface->TileType != I915_TILING_NONE)
                {
                    if (NeedSwizzleData(surface, true) == false)
                        return nullptr;
                }
            }
#else
            mos_gem_bo_map_gtt(surface->bo);
#endif
        }
        else
        {
            if (surface->TileType == I915_TILING_NONE)
            {
                mos_bo_map(surface->bo, flag & MOS_LOCKFLAG_WRITEONLY);
            }
            else if (flag & MOS_LOCKFLAG_WRITEONLY)
            {
                mos_gem_bo_map_gtt(surface->bo);
            }
            else 
            {
                mos_gem_bo_map_unsynchronized(surface->bo);     // only call mmap_gtt ioctl
                mos_gem_bo_start_gtt_access(surface->bo, 0);    // set to GTT domain,0 means readonly
            }
        }
        surface->pData   = (uint8_t*) surface->bo->virt;
        surface->bMapped = true;
    }
    else
    {
        // do nothing here
    }
    surface->iRefCount++;

    return surface->pData;
}

void DdiMediaUtil_UnlockSurface(DDI_MEDIA_SURFACE  *surface)
{
    DDI_CHK_NULL(surface, "nullptr surface", );
    DDI_CHK_NULL(surface->bo, "nullptr surface->bo", );
    if (0 == surface->iRefCount)
        return;

    if((true == surface->bMapped) && (1 == surface->iRefCount))
    {
        if (surface->pMediaCtx->bIsAtomSOC)
        {
#ifdef ANDROID
            if (surface->iWidth * surface->iHeight * 3 <= GTT_SIZE_THRESHOLD)
            {
                mos_gem_bo_unmap_gtt(surface->bo);
            }
            else
            {
                if (surface->TileType != I915_TILING_NONE)
                    NeedSwizzleData(surface, false);

                mos_bo_unmap(surface->bo);
            }
#else
            mos_gem_bo_unmap_gtt(surface->bo);
#endif
        }
        else
        {
            if (surface->TileType == I915_TILING_NONE)
            {
               mos_bo_unmap(surface->bo);
            }
            else
            {
               mos_gem_bo_unmap_gtt(surface->bo);
            }
        }
        surface->pData       = nullptr;
        surface->bo->virt    = nullptr;
        surface->bMapped     = false;
    }
    else
    {
        // do nothing here
    }

    surface->iRefCount--;

    return;
}

// add thread protection for multiple thread?
// MapBuffer?
void* DdiMediaUtil_LockBuffer(DDI_MEDIA_BUFFER *buf, uint32_t flag)
{
    DDI_CHK_NULL(buf, "nullptr buf", nullptr);
    if((Media_Format_CPU != buf->format) && (false == buf->bMapped))
    {
        if (nullptr != buf->pSurface)
        {
            DdiMediaUtil_LockSurface(buf->pSurface, flag);
            buf->pData = buf->pSurface->pData;
        }
        else
        {
            if (buf->pMediaCtx->bIsAtomSOC)
            {
                mos_gem_bo_map_gtt(buf->bo);
            }
            else
            {
                if (buf->TileType == I915_TILING_NONE)
                {
                    mos_bo_map(buf->bo, ((MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY) & flag));
                }
                else
                {
                    mos_gem_bo_map_gtt(buf->bo);
                }
             }

            buf->pData = (uint8_t*)(buf->bo->virt);
        }

        buf->bMapped = true;
        buf->iRefCount++;
    }
    else if ((Media_Format_CPU == buf->format) && (false == buf->bMapped))
    {
        buf->bMapped = true;
        buf->iRefCount++;
    }
    else 
    {
        buf->iRefCount++;
    }

    return buf->pData;
}

void DdiMediaUtil_UnlockBuffer(DDI_MEDIA_BUFFER *buf)
{
    DDI_CHK_NULL(buf, "nullptr buf", );
    if (0 == buf->iRefCount)
        return;
    if((true == buf->bMapped) && (Media_Format_CPU != buf->format) && (1 == buf->iRefCount))
    {
        if (nullptr != buf->pSurface)
        {
            DdiMediaUtil_UnlockSurface(buf->pSurface);
        }
        else
        {
             if (buf->pMediaCtx->bIsAtomSOC)
             {
                 mos_gem_bo_unmap_gtt(buf->bo);
             }
             else
             {
                 if (buf->TileType == I915_TILING_NONE)
                 {
                     mos_bo_unmap(buf->bo);
                 }
                 else
                 {
                     mos_gem_bo_unmap_gtt(buf->bo);
                 }
            }
            buf->bo->virt = nullptr;
        }

        buf->pData       = nullptr;

        buf->bMapped     = false;
    }
    else if ((true == buf->bMapped) && (Media_Format_CPU == buf->format) && (1 == buf->iRefCount))
    {
        buf->bMapped     = false;
    }
    else 
    {
        // do nothing here
    }
    buf->iRefCount--;
    return;
}

// should ref_count added for bo?
void DdiMediaUtil_FreeSurface(DDI_MEDIA_SURFACE *surface)
{ 
    DDI_CHK_NULL(surface, "nullptr surface", );
    DDI_CHK_NULL(surface->bo, "nullptr surface->bo", );
    DDI_CHK_NULL(surface->pMediaCtx, "nullptr surface->pMediaCtx", );
    DDI_CHK_NULL(surface->pMediaCtx->pGmmClientContext, "nullptr surface->pMediaCtx->pGmmClientContext", );

    // Unmap Aux mapping if the surface was mapped
    if (surface->pMediaCtx->m_auxTableMgr)
    {
        surface->pMediaCtx->m_auxTableMgr->UnmapResource(surface->pGmmResourceInfo, surface->bo);
    }

    // For External Buffer, only needs to destory SurfaceDescriptor
    if ( DdiMediaUtil_IsExternalSurface(surface) )
    {
        // In DdiMediaUtil_AllocateSurface call, driver will increase the surface reference count by calling drm_intel_bo_gem_create_from_name
        // Thus, when freeing the surface, the drm_intel_bo_unreference function should be called to avoid memory leak
        mos_bo_unreference(surface->bo);
        MOS_FreeMemory(surface->pSurfDesc);
        surface->pSurfDesc = nullptr;
    }
    else
    {
        // calling sequence checking
        if (surface->bMapped)
        {
            DdiMediaUtil_UnlockSurface(surface);
            DDI_VERBOSEMESSAGE("DDI: try to free a locked surface.");
        }
        mos_bo_unreference(surface->bo);
        surface->bo = nullptr;
    }

    if (nullptr != surface->pGmmResourceInfo)
    {
        surface->pMediaCtx->pGmmClientContext->DestroyResInfoObject(surface->pGmmResourceInfo);
        surface->pGmmResourceInfo = nullptr;
    }
}


// should ref_count added for bo?
void DdiMediaUtil_FreeBuffer(DDI_MEDIA_BUFFER  *buf)
{
    DDI_CHK_NULL(buf, "nullptr", );
    DDI_CHK_NULL(buf->pMediaCtx, "nullptr", );
    DDI_CHK_NULL(buf->pMediaCtx->pGmmClientContext, "nullptr", );
    // calling sequence checking
    if (buf->bMapped)
    {
        DdiMediaUtil_UnlockBuffer(buf);
        DDI_VERBOSEMESSAGE("DDI: try to free a locked buffer.");
    }
    if (buf->format == Media_Format_CPU)
    {
        MOS_FreeMemory(buf->pData);
        buf->pData = nullptr;
    }
    else
    {
        mos_bo_unreference(buf->bo);
        buf->bo = nullptr;
    }

    if (nullptr != buf->pGmmResourceInfo)
    {
        buf->pMediaCtx->pGmmClientContext->DestroyResInfoObject(buf->pGmmResourceInfo);
        buf->pGmmResourceInfo = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// purpose: fill a rect structure with the regsion specified by parameters
// rect[in]: input pointer to the rect
// offset_x: x offset of the region
// offset_y: y offset of the region
// width:    width of the region
// hiehgt:   height of the regsion
////////////////////////////////////////////////////////////////////////////////////////////////////
VAStatus DdiMediaUtil_FillPositionToRect(RECT *rect, int16_t offset_x, int16_t offset_y, int16_t width, int16_t height)
{
    DDI_CHK_NULL(rect, "Invalid Rect.", VA_STATUS_ERROR_INVALID_PARAMETER);

    rect->left   = offset_x;
    rect->top    = offset_y;
    rect->right  = offset_x + width;
    rect->bottom = offset_y + height;

    return VA_STATUS_SUCCESS;
}

void DdiMediaUtil_InitMutex(PMEDIA_MUTEX_T  mutex)
{
    pthread_mutex_init(mutex, nullptr);
}

void DdiMediaUtil_DestroyMutex(PMEDIA_MUTEX_T  mutex)
{
    int32_t ret = pthread_mutex_destroy(mutex);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't destroy the mutex!\n");
    }
}

void DdiMediaUtil_LockMutex(PMEDIA_MUTEX_T  mutex)
{
    int32_t ret = pthread_mutex_lock(mutex);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't lock the mutex!\n");
    }
}

void DdiMediaUtil_UnLockMutex(PMEDIA_MUTEX_T  mutex)
{
    int32_t ret = pthread_mutex_unlock(mutex);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't unlock the mutex!\n");
    }
}

void DdiMediaUtil_DestroySemaphore(PMEDIA_SEM_T  sem)
{
    int32_t ret = sem_destroy(sem);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't destroy the semaphore!\n");
    }
}

void DdiMediaUtil_WaitSemaphore(PMEDIA_SEM_T  sem)
{
    int32_t ret = sem_wait(sem);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("wait semaphore error!\n");
    }
}

int32_t DdiMediaUtil_TryWaitSemaphore(PMEDIA_SEM_T  sem)
{
    return sem_trywait(sem);
}

void DdiMediaUtil_PostSemaphore(PMEDIA_SEM_T  sem)
{
    int32_t ret = sem_post(sem);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("post semaphore error!\n");
    }
}

// heap related
PDDI_MEDIA_SURFACE_HEAP_ELEMENT DdiMediaUtil_AllocPMediaSurfaceFromHeap(PDDI_MEDIA_HEAP surfaceHeap)
{
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  mediaSurfaceHeapElmt;

    if (nullptr == surfaceHeap->pFirstFreeHeapElement)
    {
        void *newHeapBase = MOS_ReallocMemory(surfaceHeap->pHeapBase, (surfaceHeap->uiAllocatedHeapElements + DDI_MEDIA_HEAP_INCREMENTAL_SIZE) * sizeof(DDI_MEDIA_SURFACE_HEAP_ELEMENT));

        if (nullptr == newHeapBase)
        {
            DDI_ASSERTMESSAGE("DDI: realloc failed.");
            return nullptr;
        }
        surfaceHeap->pHeapBase                    = newHeapBase;
        PDDI_MEDIA_SURFACE_HEAP_ELEMENT surfaceHeapBase  = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surfaceHeap->pHeapBase;
        surfaceHeap->pFirstFreeHeapElement        = (void*)(&surfaceHeapBase[surfaceHeap->uiAllocatedHeapElements]);
        for (int32_t i = 0; i < (DDI_MEDIA_HEAP_INCREMENTAL_SIZE); i++)
        {
            mediaSurfaceHeapElmt                  = &surfaceHeapBase[surfaceHeap->uiAllocatedHeapElements + i];
            mediaSurfaceHeapElmt->pNextFree       = (i == (DDI_MEDIA_HEAP_INCREMENTAL_SIZE - 1))? nullptr : &surfaceHeapBase[surfaceHeap->uiAllocatedHeapElements + i + 1];
            mediaSurfaceHeapElmt->uiVaSurfaceID   = surfaceHeap->uiAllocatedHeapElements + i;
        }
        surfaceHeap->uiAllocatedHeapElements     += DDI_MEDIA_HEAP_INCREMENTAL_SIZE;
    }

    mediaSurfaceHeapElmt                          = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surfaceHeap->pFirstFreeHeapElement;
    surfaceHeap->pFirstFreeHeapElement            = mediaSurfaceHeapElmt->pNextFree;

    return mediaSurfaceHeapElmt;
}


void DdiMediaUtil_ReleasePMediaSurfaceFromHeap(PDDI_MEDIA_HEAP surfaceHeap, uint32_t vaSurfaceID)
{
    DDI_CHK_LESS(vaSurfaceID, surfaceHeap->uiAllocatedHeapElements, "invalid surface id", );
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapBase                   = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surfaceHeap->pHeapBase;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapElmt                   = &mediaSurfaceHeapBase[vaSurfaceID];
    DDI_CHK_NULL(mediaSurfaceHeapElmt->pSurface, "surface is already released", );
    void *firstFree                         = surfaceHeap->pFirstFreeHeapElement;
    surfaceHeap->pFirstFreeHeapElement     = (void*)mediaSurfaceHeapElmt;
    mediaSurfaceHeapElmt->pNextFree        = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)firstFree;
    mediaSurfaceHeapElmt->pSurface         = nullptr;
}


PDDI_MEDIA_BUFFER_HEAP_ELEMENT DdiMediaUtil_AllocPMediaBufferFromHeap(PDDI_MEDIA_HEAP bufferHeap)
{
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT  mediaBufferHeapElmt;
    if (nullptr == bufferHeap->pFirstFreeHeapElement)
    {
        void *newHeapBase = MOS_ReallocMemory(bufferHeap->pHeapBase, (bufferHeap->uiAllocatedHeapElements + DDI_MEDIA_HEAP_INCREMENTAL_SIZE) * sizeof(DDI_MEDIA_BUFFER_HEAP_ELEMENT));
        if (nullptr == newHeapBase)
        {
            DDI_ASSERTMESSAGE("DDI: realloc failed.");
            return nullptr;
        }
        bufferHeap->pHeapBase                                 = newHeapBase;
        PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapBase    = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)bufferHeap->pHeapBase;
        bufferHeap->pFirstFreeHeapElement     = (void*)(&mediaBufferHeapBase[bufferHeap->uiAllocatedHeapElements]);
        for (int32_t i = 0; i < (DDI_MEDIA_HEAP_INCREMENTAL_SIZE); i++)
        {
            mediaBufferHeapElmt               = &mediaBufferHeapBase[bufferHeap->uiAllocatedHeapElements + i];
            mediaBufferHeapElmt->pNextFree    = (i == (DDI_MEDIA_HEAP_INCREMENTAL_SIZE - 1))? nullptr : &mediaBufferHeapBase[bufferHeap->uiAllocatedHeapElements + i + 1];
            mediaBufferHeapElmt->uiVaBufferID = bufferHeap->uiAllocatedHeapElements + i;
        }
        bufferHeap->uiAllocatedHeapElements  += DDI_MEDIA_HEAP_INCREMENTAL_SIZE;
    }

    mediaBufferHeapElmt                       = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)bufferHeap->pFirstFreeHeapElement;
    bufferHeap->pFirstFreeHeapElement         = mediaBufferHeapElmt->pNextFree;
    return mediaBufferHeapElmt;
}


void DdiMediaUtil_ReleasePMediaBufferFromHeap(PDDI_MEDIA_HEAP bufferHeap, uint32_t vaBufferID)
{
    DDI_CHK_LESS(vaBufferID, bufferHeap->uiAllocatedHeapElements, "invalid buffer id", );
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapBase                    = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)bufferHeap->pHeapBase;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapElmt                    = &mediaBufferHeapBase[vaBufferID];
    DDI_CHK_NULL(mediaBufferHeapElmt->pBuffer, "buffer is already released", );
    void *firstFree                        = bufferHeap->pFirstFreeHeapElement;
    bufferHeap->pFirstFreeHeapElement      = (void*)mediaBufferHeapElmt;
    mediaBufferHeapElmt->pNextFree         = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)firstFree;
    mediaBufferHeapElmt->pBuffer           = nullptr;
}

PDDI_MEDIA_IMAGE_HEAP_ELEMENT DdiMediaUtil_AllocPVAImageFromHeap(PDDI_MEDIA_HEAP imageHeap)
{
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT   vaimageHeapElmt;

    if (nullptr == imageHeap->pFirstFreeHeapElement)
    {
        void *newHeapBase = MOS_ReallocMemory(imageHeap->pHeapBase, (imageHeap->uiAllocatedHeapElements + DDI_MEDIA_HEAP_INCREMENTAL_SIZE) * sizeof(DDI_MEDIA_IMAGE_HEAP_ELEMENT));

        if (nullptr == newHeapBase)
        {
            DDI_ASSERTMESSAGE("DDI: realloc failed.");
            return nullptr;
        }
        imageHeap->pHeapBase                           = newHeapBase;
        PDDI_MEDIA_IMAGE_HEAP_ELEMENT vaimageHeapBase  = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)imageHeap->pHeapBase;
        imageHeap->pFirstFreeHeapElement               = (void*)(&vaimageHeapBase[imageHeap->uiAllocatedHeapElements]);
        for (int32_t i = 0; i < (DDI_MEDIA_HEAP_INCREMENTAL_SIZE); i++)
        {
            vaimageHeapElmt                   = &vaimageHeapBase[imageHeap->uiAllocatedHeapElements + i];
            vaimageHeapElmt->pNextFree        = (i == (DDI_MEDIA_HEAP_INCREMENTAL_SIZE - 1))? nullptr : &vaimageHeapBase[imageHeap->uiAllocatedHeapElements + i + 1];
            vaimageHeapElmt->uiVaImageID      = imageHeap->uiAllocatedHeapElements + i;
        }
        imageHeap->uiAllocatedHeapElements   += DDI_MEDIA_HEAP_INCREMENTAL_SIZE;

    }

    vaimageHeapElmt                           = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)imageHeap->pFirstFreeHeapElement;
    imageHeap->pFirstFreeHeapElement          = vaimageHeapElmt->pNextFree;
    return vaimageHeapElmt;
}


void DdiMediaUtil_ReleasePVAImageFromHeap(PDDI_MEDIA_HEAP imageHeap, uint32_t vaImageID)
{
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT    vaImageHeapBase;
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT    vaImageHeapElmt;
    void                            *firstFree;

    DDI_CHK_LESS(vaImageID, imageHeap->uiAllocatedHeapElements, "invalid image id", );
    vaImageHeapBase                    = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)imageHeap->pHeapBase;
    vaImageHeapElmt                    = &vaImageHeapBase[vaImageID];
    DDI_CHK_NULL(vaImageHeapElmt->pImage, "image is already released", );
    firstFree                          = imageHeap->pFirstFreeHeapElement;
    imageHeap->pFirstFreeHeapElement   = (void*)vaImageHeapElmt;
    vaImageHeapElmt->pNextFree         = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)firstFree;
    vaImageHeapElmt->pImage            = nullptr;
}

PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT DdiMediaUtil_AllocPVAContextFromHeap(PDDI_MEDIA_HEAP vaContextHeap)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT   vacontextHeapElmt;
    if (nullptr == vaContextHeap->pFirstFreeHeapElement)
    {
        void *newHeapBase = MOS_ReallocMemory(vaContextHeap->pHeapBase, (vaContextHeap->uiAllocatedHeapElements + DDI_MEDIA_HEAP_INCREMENTAL_SIZE) * sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT));

        if (nullptr == newHeapBase)
        {
            DDI_ASSERTMESSAGE("DDI: realloc failed.");
            return nullptr;
        }
        vaContextHeap->pHeapBase                            = newHeapBase;
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vacontextHeapBase = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)vaContextHeap->pHeapBase;
        vaContextHeap->pFirstFreeHeapElement        = (void*)(&(vacontextHeapBase[vaContextHeap->uiAllocatedHeapElements]));
        for (int32_t i = 0; i < (DDI_MEDIA_HEAP_INCREMENTAL_SIZE); i++)
        {
            vacontextHeapElmt                       = &vacontextHeapBase[vaContextHeap->uiAllocatedHeapElements + i];
            vacontextHeapElmt->pNextFree            = (i == (DDI_MEDIA_HEAP_INCREMENTAL_SIZE - 1))? nullptr : &vacontextHeapBase[vaContextHeap->uiAllocatedHeapElements + i + 1];
            vacontextHeapElmt->uiVaContextID        = vaContextHeap->uiAllocatedHeapElements + i;
            vacontextHeapElmt->pVaContext           = nullptr;
        }
        vaContextHeap->uiAllocatedHeapElements     += DDI_MEDIA_HEAP_INCREMENTAL_SIZE;
    }

    vacontextHeapElmt                               = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)vaContextHeap->pFirstFreeHeapElement;
    vaContextHeap->pFirstFreeHeapElement            = vacontextHeapElmt->pNextFree;
    return vacontextHeapElmt;
}


void DdiMediaUtil_ReleasePVAContextFromHeap(PDDI_MEDIA_HEAP vaContextHeap, uint32_t vaContextID)
{
    DDI_CHK_LESS(vaContextID, vaContextHeap->uiAllocatedHeapElements, "invalid context id", );
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaContextHeapBase = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)vaContextHeap->pHeapBase;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaContextHeapElmt = &vaContextHeapBase[vaContextID];
    DDI_CHK_NULL(vaContextHeapElmt->pVaContext, "context is already released", );
    void *firstFree                        = vaContextHeap->pFirstFreeHeapElement;
    vaContextHeap->pFirstFreeHeapElement   = (void*)vaContextHeapElmt;
    vaContextHeapElmt->pNextFree           = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)firstFree;
    vaContextHeapElmt->pVaContext          = nullptr;
}

void DdiMediaUtil_UnRefBufObjInMediaBuffer(PDDI_MEDIA_BUFFER buf)
{
    mos_bo_unreference(buf->bo);
}

// Open Intel's Graphics Device to get the file descriptor
int32_t DdiMediaUtil_OpenGraphicsAdaptor(char *devName)
{
    struct stat st;
    int32_t    hDevice = -1;
    if(nullptr == devName)
    {
        DDI_ASSERTMESSAGE("Invalid Graphics Node");
        return -1;
    }

    if (-1 == stat (devName, &st))
    {
        DDI_ASSERTMESSAGE("Cannot identify '%s': %d, %s.", devName, errno, strerror (errno));
        return -1;
    }

    if (!S_ISCHR (st.st_mode))
    {
        DDI_ASSERTMESSAGE("%s is no device.", devName);
        return -1;
    }

    hDevice = open (devName, O_RDWR);
    if (-1 == hDevice)
    {
        DDI_ASSERTMESSAGE("Cannot open '%s': %d, %s.", devName, errno, strerror (errno));
        return -1;
    }

    return hDevice;
}

VAStatus DdiMediaUtil_UnRegisterRTSurfaces(
    VADriverContextP    ctx,
    PDDI_MEDIA_SURFACE surface)
{
    DDI_CHK_NULL(ctx,"nullptr context!", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,"nullptr mediaCtx!", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(surface, "nullptr surface!", VA_STATUS_ERROR_INVALID_PARAMETER);

    //Look through all decode contexts to unregister the surface in each decode context's RTtable.
    if (mediaCtx->pDecoderCtxHeap != nullptr)
    {
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT decVACtxHeapBase;

        DdiMediaUtil_LockMutex(&mediaCtx->DecoderMutex);
        decVACtxHeapBase  = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)mediaCtx->pDecoderCtxHeap->pHeapBase;
        for (int32_t j = 0; j < mediaCtx->pDecoderCtxHeap->uiAllocatedHeapElements; j++)
        {
            if (decVACtxHeapBase[j].pVaContext != nullptr)
            {
                PDDI_DECODE_CONTEXT  decCtx = (PDDI_DECODE_CONTEXT)decVACtxHeapBase[j].pVaContext;
                if (decCtx && decCtx->m_ddiDecode)
                {
                    //not check the return value since the surface may not be registered in the context. pay attention to LOGW.
                    decCtx->m_ddiDecode->UnRegisterRTSurfaces(&decCtx->RTtbl, surface);
                }
            }
        }
        DdiMediaUtil_UnLockMutex(&mediaCtx->DecoderMutex);
    }
    if (mediaCtx->pEncoderCtxHeap != nullptr)
    {
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT pEncVACtxHeapBase;

        DdiMediaUtil_LockMutex(&mediaCtx->EncoderMutex);
        pEncVACtxHeapBase  = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)mediaCtx->pEncoderCtxHeap->pHeapBase;
        for (int32_t j = 0; j < mediaCtx->pEncoderCtxHeap->uiAllocatedHeapElements; j++)
        {
            if (pEncVACtxHeapBase[j].pVaContext != nullptr)
            {
                PDDI_ENCODE_CONTEXT  pEncCtx = (PDDI_ENCODE_CONTEXT)pEncVACtxHeapBase[j].pVaContext;
                if (pEncCtx && pEncCtx->m_encode)
                {
                    //not check the return value since the surface may not be registered in the context. pay attention to LOGW.
                    pEncCtx->m_encode->UnRegisterRTSurfaces(&pEncCtx->RTtbl, surface);
                }
            }
        }
        DdiMediaUtil_UnLockMutex(&mediaCtx->EncoderMutex);
    }

    return VA_STATUS_SUCCESS;
}
