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

#ifdef ANDROID
#include <va/va_android.h>
#include <ufo/gralloc.h>
#endif

#include "media_libva_util.h"
#include "mos_utilities.h" 
#include "mos_os.h"
#include "hwinfo_linux.h"

#ifdef ANDROID
#define NV12_DUMP_FILE_NAME "/mnt/sdcard/nv12.yuv"
#else
#define NV12_DUMP_FILE_NAME "./nv12.yuv"
#endif

#define DDI_UTIL_CHK_NULL(p) (p == nullptr )

#ifdef DEBUG
static int32_t         g_iFrameCountFps   = -1;
static struct timeval  g_Tv1;
static pthread_mutex_t g_FpsMutex         = PTHREAD_MUTEX_INITIALIZER;
static int32_t         g_iVAFpsSampleSize = 100;

#define LENGTH_OF_FPS_FILE_NAME 128

static int32_t g_iCount = 0;

#ifdef ANDROID
#define FPS_FILE_NAME   "/mnt/sdcard/fps.txt"
#else
#define FPS_FILE_NAME   "./fps.txt"
#endif
#endif

/*----------------------------------------------------------------------------
| Name      : DdiMediaUtil_MediaDumpNV12
|
| Purpose   : Dump out the NV12 surface and save in file
|
| Arguments : [in] pSrcY        - Pointer to Luminance
|             [in] pSrcUV       - Pointer to interleaved chrominance
|             [in] iWidth       - Frame width
|             [in] iHeight      - Frame height
|             [in] iPitch       - Pitch for the frame store
|             [in] iFrameNum    - The number of frames to be dumped out
|
| Returns   : 
| 
| Comments  : The dumped out yuv is in YV12 format, can be opened by yuv viewer
\---------------------------------------------------------------------------*/
#ifdef DEBUG
void DdiMediaUtil_MediaDumpNV12(
    uint8_t *pSrcY, 
    uint8_t *pSrcUV, 
    int32_t  iWidth, 
    int32_t  iHeight, 
    int32_t  iPitch, 
    int32_t  iFrameNum)
{
    FILE    *fp;
    uint8_t *pUV, *pY;
    int32_t  i;

    if(g_iCount >= iFrameNum)
    {
        return;
    }

    if(0 == g_iCount)
        fp = fopen(NV12_DUMP_FILE_NAME,"wb");
    else
        fp = fopen(NV12_DUMP_FILE_NAME,"ab");

    if(fp == nullptr)
    {
        return;
    }

    // Y
    pY = pSrcY;
    for(i = 0; i < iHeight; i++, pY += iPitch)
    {
        fwrite(pY, 1, iWidth, fp);
    }

    // U
    pUV = pSrcUV;
    for(i = 0; i < (iHeight/2); i++, pUV += iPitch)
    {
        int32_t j;
        for(j = 0; j < (iWidth/2); j++)
        {
            fwrite(&pUV[2*j+0], 1, sizeof(pUV[0]), fp);
        }
    }

    // V
    pUV = pSrcUV;
    for(i = 0; i < (iHeight/2); i++, pUV += iPitch)
    {
        int32_t j;
        for(j = 0; j < (iWidth/2); j++)
        {
            fwrite(&pUV[2*j+1], 1, sizeof(pUV[0]), fp);
        }
    }

    g_iCount++;
    fclose(fp);
}

void DdiMediaUtil_MediaPrintFps()
{
    struct timeval Tv2;
    int64_t        i64Diff;
    float          fFps;

    if (0 == g_iVAFpsSampleSize) 
    {
        return;
    }
    gettimeofday(&Tv2, 0);

    pthread_mutex_lock(&g_FpsMutex);
    if (-1 == g_iFrameCountFps) 
    {
        gettimeofday(&g_Tv1, 0);
    }

    if (++g_iFrameCountFps >= g_iVAFpsSampleSize) 
    {
        char   cFpsFileName[LENGTH_OF_FPS_FILE_NAME];
        FILE   *fp;
        char   cTemp[LENGTH_OF_FPS_FILE_NAME];

        fp      = nullptr;
        i64Diff = (Tv2.tv_sec - g_Tv1.tv_sec)*1000000 + Tv2.tv_usec - g_Tv1.tv_usec;
        fFps    = g_iFrameCountFps / (i64Diff / 1000000.0);
        DDI_NORMALMESSAGE("FPS:%6.4f, Interval:%11lu.", fFps,((uint64_t)Tv2.tv_sec)*1000 + (Tv2.tv_usec/1000));
        sprintf(cTemp,"FPS:%6.4f, Interval:%11lu\n", fFps,((uint64_t)Tv2.tv_sec)*1000 + (Tv2.tv_usec/1000));

        MOS_ZeroMemory(cFpsFileName,LENGTH_OF_FPS_FILE_NAME);
        sprintf(cFpsFileName, FPS_FILE_NAME);
        if ((fp = fopen(cFpsFileName, "wb")) == nullptr) 
        {
            pthread_mutex_unlock(&g_FpsMutex);
            DDI_ASSERTMESSAGE("Unable to open fps file.");
        }

        fwrite(cTemp, 1, strlen(cTemp), fp);
        fclose(fp);
        g_iFrameCountFps = -1;
    }
    pthread_mutex_unlock(&g_FpsMutex);
}
#else
void DdiMediaUtil_MediaDumpNV12(
    uint8_t *pSrcY, 
    uint8_t *pSrcUV, 
    int32_t  iWidth, 
    int32_t  iHeight, 
    int32_t  iPitch, 
    int32_t  iFrameNum)
{
    DDI_UNUSED(pSrcY);
    DDI_UNUSED(pSrcUV);
    DDI_UNUSED(iWidth);
    DDI_UNUSED(iHeight);
    DDI_UNUSED(iPitch);
    DDI_UNUSED(iFrameNum);
    return;
}

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
    int32_t             CsxSwizzle)     // (Boolean) Additionally perform Channel Select XOR swizzling.
{
    int32_t Row, Line, Col, x; // Linear Offset Components
    int32_t LBits, LPos; // Size and swizzled position of the Line component.
    int32_t SwizzledOffset;
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

    Row =   OffsetY >> LBits;               // OffsetY / LinesPerTile
    Line =  OffsetY & ((1 << LBits) - 1);   // OffsetY % LinesPerTile
    Col =   OffsetX >> LPos;                // OffsetX / BytesPerLine
    x =     OffsetX & ((1 << LPos) - 1);    // OffsetX % BytesPerLine

    SwizzledOffset =
        (((((Row * (Pitch >> LPos)) + Col) << LBits) + Line) << LPos) + x;
    //                V                V                 V
    //                / BytesPerLine   * LinesPerTile    * BytesPerLine

    /// Channel Select XOR Swizzling ///////////////////////////////////////////
    if (CsxSwizzle)
    {
        if (TileFormat == I915_TILING_Y) // A6 = A6 ^ A9
        {
            SwizzledOffset ^= ((SwizzledOffset >> (9 - 6)) & 0x40);
        }
        else //if (TileFormat == I915_TILING_X) // A6 = A6 ^ A9 ^ A10
        {
            SwizzledOffset ^= (((SwizzledOffset >> (9 - 6)) ^ (SwizzledOffset >> (10 - 6))) & 0x40);
        }
    }

    return(SwizzledOffset);
}

static void SwizzleData(
    uint8_t  *pSrc,
    uint8_t  *pDst,
    uint32_t  SrcTiling,
    uint32_t  DstTiling,
    int32_t   iHeight,
    int32_t   iPitch)
{
#define IS_TILED(_a)                ((_a) != I915_TILING_NONE)
#define IS_TILED_TO_LINEAR(_a, _b)  (IS_TILED(_a) && !IS_TILED(_b))
#define IS_LINEAR_TO_TILED(_a, _b)  (!IS_TILED(_a) && IS_TILED(_b))

    int32_t LinearOffset;
    int32_t TileOffset;
    int32_t x;
    int32_t y;

    // Translate from one format to another
    for (y = 0, LinearOffset = 0, TileOffset = 0; y < iHeight; y++)
    {
        for (x = 0; x < iPitch; x++, LinearOffset++)
        {
            // x or y --> linear
            if (IS_TILED_TO_LINEAR(SrcTiling, DstTiling))
            {
                TileOffset = SwizzleOffset(
                    x,
                    y,
                    iPitch,
                    SrcTiling,
                    false);

                *(pDst + LinearOffset) = *(pSrc + TileOffset);
            }
            // linear --> x or y
            else if (IS_LINEAR_TO_TILED(SrcTiling, DstTiling))
            {
                TileOffset = SwizzleOffset(
                    x,
                    y,
                    iPitch,
                    DstTiling,
                    false);

                *(pDst + TileOffset) = *(pSrc + LinearOffset);
            }
            else
            {
                MOS_OS_ASSERT(0);
            }
        }
    }
}

static bool NeedSwizzleData(PDDI_MEDIA_SURFACE pSurface, bool bLock)
{
    uint32_t            iSize, iPitch;
    uint8_t            *pResourceBase;
    GMM_RESOURCE_FLAG   GmmFlags;
	DDI_CHK_NULL(pSurface, "nullptr pSurface", false);
	DDI_CHK_NULL(pSurface->pGmmResourceInfo, "nullptr pGmmResourceInfo", false);
    iPitch = (uint32_t)pSurface->pGmmResourceInfo->GetRenderPitch();
    iSize  = GmmResGetRenderSize(pSurface->pGmmResourceInfo);
    GmmFlags = pSurface->pGmmResourceInfo->GetResFlags();

    if (GmmFlags.Gpu.RenderTarget      &&
        GmmFlags.Gpu.UnifiedAuxSurface &&
        GmmFlags.Gpu.CCS)
    {
        iSize = iSize - (uint32_t)(pSurface->pGmmResourceInfo->GetSizeAuxSurface(GMM_AUX_SURF));
    }

    pResourceBase = (uint8_t*)MOS_AllocAndZeroMemory(iSize);
    if(DDI_UTIL_CHK_NULL(pResourceBase))
       return false;

    if (bLock)
    {
        SwizzleData((uint8_t*) pSurface->bo->virt, pResourceBase, pSurface->TileType, I915_TILING_NONE, iSize / iPitch, iPitch);
    }
    else
    {
        SwizzleData((uint8_t*) pSurface->bo->virt, pResourceBase, I915_TILING_NONE, pSurface->TileType, iSize / iPitch, iPitch);
    }
    MOS_SecureMemcpy((uint8_t*) pSurface->bo->virt, iSize, pResourceBase, iSize);
    MOS_FreeMemory(pResourceBase);

    return true;
}
#endif
/*
 * DdiMediaUtil_IsExternalSurface
 *    Descripion: if the bo of media surface was allocated from App,
 *                should return true, otherwise, false. In current implemeation 
 *                external buffer passed with pSurfDesc.
*/
bool DdiMediaUtil_IsExternalSurface(PDDI_MEDIA_SURFACE pSurface)
{
    if ( nullptr == pSurface )
    {
        return false;
    }
    else if ( pSurface->pSurfDesc == nullptr )
    {
        return false;
    }

    return true;
}

/*
 * DdiMediaUtil_ConvertMediaFmtToGmmFmt
 *    Descripion: convert Media Format to Gmm Format for GmmResCreate parameter.
*/
static GMM_RESOURCE_FORMAT DdiMediaUtil_ConvertMediaFmtToGmmFmt(
    DDI_MEDIA_FORMAT format)
{
    switch (format)
    {
        case Media_Format_X8R8G8B8   : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case Media_Format_A8R8G8B8   : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case Media_Format_X8B8G8R8   : return GMM_FORMAT_R8G8B8X8_UNORM_TYPE;
        case Media_Format_A8B8G8R8   : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case Media_Format_R5G6B5     : return GMM_FORMAT_B5G6R5_UNORM_TYPE;
        case Media_Format_R8G8B8     : return GMM_FORMAT_R8G8B8_UNORM;
        case Media_Format_NV12       : return GMM_FORMAT_NV12_TYPE;
        case Media_Format_NV21       : return GMM_FORMAT_NV21_TYPE;
        case Media_Format_YUY2       : return GMM_FORMAT_YUY2;
        case Media_Format_UYVY       : return GMM_FORMAT_UYVY;
        case Media_Format_YV12       : return GMM_FORMAT_YV12_TYPE;
        case Media_Format_IYUV       : return GMM_FORMAT_IYUV_TYPE;
        case Media_Format_I420       : return GMM_FORMAT_I420_TYPE;
        case Media_Format_444P       : return GMM_FORMAT_MFX_JPEG_YUV444_TYPE;
        case Media_Format_422H       : return GMM_FORMAT_MFX_JPEG_YUV422H_TYPE;
        case Media_Format_411P       : return GMM_FORMAT_MFX_JPEG_YUV411_TYPE;
        case Media_Format_422V       : return GMM_FORMAT_MFX_JPEG_YUV422V_TYPE;
        case Media_Format_IMC3       : return GMM_FORMAT_IMC3_TYPE;
        case Media_Format_400P       : return GMM_FORMAT_GENERIC_8BIT;
        case Media_Format_Buffer     : return GMM_FORMAT_RENDER_8BIT;
        case Media_Format_P010       : return GMM_FORMAT_P010_TYPE;
        case Media_Format_R10G10B10A2: return GMM_FORMAT_R10G10B10A2_UNORM_TYPE;
        case Media_Format_B10G10R10A2: return GMM_FORMAT_B10G10R10A2_UNORM_TYPE;   
        default                      : return GMM_FORMAT_INVALID;
    }
}

VAStatus DdiMediaUtil_AllocateSurface(
    DDI_MEDIA_FORMAT            Format,
    int32_t                     iWidth,
    int32_t                     iHeight,
    PDDI_MEDIA_SURFACE          pMediaSurface,
    PDDI_MEDIA_CONTEXT          pMediaDrvCtx)
{
    int32_t                     iSize;
    int32_t                     iAlignedHeight;
    unsigned long               ulPitch;
    int32_t                     iPitch = 0;
    VAStatus                    hRes;
    MOS_LINUX_BO               *bo = nullptr;
    uint32_t                    tileformat;
    uint32_t                    gmmPitch;
    uint32_t                    gmmSize;
    uint32_t                    gmmHeight;
    uint32_t                    swizzle;
    GMM_RESCREATE_PARAMS        GmmParams;
    GMM_RESOURCE_INFO          *pGmmResourceInfo;
    bool                        bGrallocAllocation;
    __GMM_BUFFER_TYPE           gmmRestrictions = {0};
    unsigned long               ulPitchAlign;
    unsigned long               ulSizeAlign;

    DDI_CHK_NULL(pMediaSurface, "pMediaSurface is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(pMediaDrvCtx, "pMediaDrvCtx is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);

    iSize          = 0;
    tileformat     = I915_TILING_NONE;
    hRes           = VA_STATUS_SUCCESS;
    iAlignedHeight = iHeight;

    switch (Format)
    {
        case Media_Format_X8R8G8B8:
        case Media_Format_X8B8G8R8:
        case Media_Format_A8B8G8R8:
        case Media_Format_R5G6B5:
        case Media_Format_R8G8B8:
        case Media_Format_R10G10B10A2:
        case Media_Format_B10G10R10A2: 
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != pMediaSurface->surfaceUsageHint)
            {
                 tileformat = I915_TILING_NONE;
                 break;
            }
        case Media_Format_NV21:
        case Media_Format_YV12:
        case Media_Format_I420:
        case Media_Format_IYUV:
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != pMediaSurface->surfaceUsageHint)
            {
                 tileformat = I915_TILING_NONE;
                 break;
            }
        case Media_Format_YUY2:
        case Media_Format_UYVY:
        case Media_Format_A8R8G8B8:
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != pMediaSurface->surfaceUsageHint &&
                !(pMediaSurface->surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_DECODER))
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
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != pMediaSurface->surfaceUsageHint)
            {
#if UFO_GRALLOC_NEW_FORMAT
                 //Planar type surface align 64 to improve performance.
                iAlignedHeight = MOS_ALIGN_CEIL(iHeight, 64);
#else
                //Planar type surface align 32 to improve performance.
                iAlignedHeight = MOS_ALIGN_CEIL(iHeight, 32);
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

    if( DdiMediaUtil_IsExternalSurface(pMediaSurface) )
    { 
        // DRM buffer allocated by Application, No need to re-allocate new DRM buffer
         if( (pMediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM)
             || (pMediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME)
#ifdef ANDROID
             ||(pMediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC)
#endif 
           )
        {
            if (pMediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM)
            {
                bo = mos_bo_gem_create_from_name(pMediaDrvCtx->pDrmBufMgr, "MEDIA", pMediaSurface->pSurfDesc->ulBuffer);
            }
#ifdef ANDROID
            else if (pMediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC)
            {
#if INTEL_UFO_GRALLOC_HAVE_PRIME
                bo = mos_bo_gem_create_from_prime(pMediaDrvCtx->pDrmBufMgr, pMediaSurface->pSurfDesc->ulBuffer, pMediaSurface->pSurfDesc->uiSize);
#else
                bo = mos_bo_gem_create_from_name(pMediaDrvCtx->pDrmBufMgr, "MEDIA", pMediaSurface->pSurfDesc->ulBuffer);
#endif
            }
#endif
            else
            {
                bo = mos_bo_gem_create_from_prime(pMediaDrvCtx->pDrmBufMgr, pMediaSurface->pSurfDesc->ulBuffer, pMediaSurface->pSurfDesc->uiSize);
            }

            if( bo != nullptr )
            {
                uint32_t swizzle_mode;

                iPitch = pMediaSurface->pSurfDesc->uiPitches[0];

                //Overwirte the tileformat matches with the right buffer
                mos_bo_get_tiling(bo, &tileformat, &swizzle_mode);
            }
            else
            {
                DDI_ASSERTMESSAGE("Failed to create drm buffer object according to input buffer descriptor.");
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
        }
        else if( pMediaSurface->pSurfDesc->uiFlags & VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR )
        {

            iPitch    = pMediaSurface->pSurfDesc->uiPitches[0];

#ifdef ANDROID
#ifdef DRM_IOCTL_I915_GEM_USERPTR
            bo = mos_bo_alloc_userptr( pMediaDrvCtx->pDrmBufMgr,
                                          "SysSurface",
                                          (void *)pMediaSurface->pSurfDesc->ulBuffer,
                                          pMediaSurface->pSurfDesc->uiTile,
                                          iPitch,
                                          pMediaSurface->pSurfDesc->uiBuffserSize,
                                          I915_USERPTR_UNSYNCHRONIZED
                                         );
#else
            bo = mos_bo_alloc_vmap( pMediaDrvCtx->pDrmBufMgr,
                                          "SysSurface",
                                          (void *)pMediaSurface->pSurfDesc->ulBuffer,
                                          pMediaSurface->pSurfDesc->uiTile,
                                          iPitch,
                                          pMediaSurface->pSurfDesc->uiBuffserSize,
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
            DDI_ASSERTMESSAGE("Input buffer descriptor (%d) is not supported by current driver.", pMediaSurface->pSurfDesc->uiFlags);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }

    bGrallocAllocation = false;
    if( DdiMediaUtil_IsExternalSurface(pMediaSurface) )
    {        
        bGrallocAllocation = pMediaSurface->pSurfDesc->bIsGralloc;
    }
    
    if( bGrallocAllocation )    
    {
        GmmParams = pMediaSurface->pSurfDesc->GmmParam;
    }    
    else    
    {  
        // Create GmmResourceInfo
        MOS_ZeroMemory(&GmmParams, sizeof(GmmParams));
        if (DdiMediaUtil_IsExternalSurface(pMediaSurface))
        {
            GmmParams.BaseWidth         = pMediaSurface->iWidth;
            GmmParams.BaseHeight        = pMediaSurface->iHeight;
        }
        else
        {
            GmmParams.BaseWidth             = iWidth;
            GmmParams.BaseHeight            = iAlignedHeight;
        }
        
        GmmParams.ArraySize             = 1;
        GmmParams.Type                  = RESOURCE_2D;
        GmmParams.Format                = DdiMediaUtil_ConvertMediaFmtToGmmFmt(Format);
        
        DDI_CHK_CONDITION(GmmParams.Format == GMM_FORMAT_INVALID, 
                             "Unsupported format", 
                             VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT);
    }
    
    switch (tileformat)
    {
        case I915_TILING_Y:
            GmmParams.Flags.Info.TiledY    = true;

            // For surface allocated by Gralloc, we use intel_ufo_bo_type_t to store mmc state.
            if (!bGrallocAllocation)
            {
                // Disable MMC for application required surfaces, because some cases' output streams have corruption.
                GmmParams.Flags.Gpu.MMC    = false;
            }
            break;
        case I915_TILING_X:
            GmmParams.Flags.Info.TiledX    = true;
            break;
        default:
            GmmParams.Flags.Info.Linear    = true;
    }
       
    GmmParams.Flags.Gpu.Video = true;

    pMediaSurface->pGmmResourceInfo = pGmmResourceInfo = GmmResCreate(&GmmParams);

    if(nullptr == pGmmResourceInfo)
    {
        DDI_ASSERTMESSAGE("Gmm Create Resource Failed.");
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    gmmPitch    = (uint32_t)pGmmResourceInfo->GetRenderPitch();
    gmmSize     = GmmResGetRenderSize(pGmmResourceInfo);
    gmmHeight   = pGmmResourceInfo->GetBaseHeight();

    if ( 0 == gmmPitch || 0 == gmmSize || 0 == gmmHeight)
    {
        DDI_ASSERTMESSAGE("Gmm Create Resource Failed.");
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    if (!DdiMediaUtil_IsExternalSurface(pMediaSurface))
    {
#if defined(I915_PARAM_CREATE_VERSION)
        int32_t value ;
        int32_t ret;
        value = 0;
        ret = -1;
        drm_i915_getparam_t gp;
        memset( &gp, 0, sizeof(gp) );
        gp.value = &value;
        gp.param = I915_PARAM_CREATE_VERSION;
        ret = drmIoctl(pMediaDrvCtx->fd, DRM_IOCTL_I915_GETPARAM, &gp);
        if ((0 == ret) && (tileformat != I915_TILING_NONE))
        {
            bo = mos_bo_alloc_tiled(pMediaDrvCtx->pDrmBufMgr, "MEDIA", gmmPitch, gmmSize/gmmPitch, 1, &tileformat, (unsigned long *)&ulPitch, BO_ALLOC_STOLEN);
            if (nullptr == bo)
            {
                bo = mos_bo_alloc_tiled(pMediaDrvCtx->pDrmBufMgr, "MEDIA", gmmPitch, gmmSize/gmmPitch, 1, &tileformat, (unsigned long *)&ulPitch, 0);
            }
            else
            {
                DDI_VERBOSEMESSAGE("Stolen memory is created sucessfully on AllocateSurface");
            }
            iPitch = (int32_t)ulPitch;
        }
        else
#endif  
        {
            if ( tileformat == I915_TILING_NONE )
            {
                bo = mos_bo_alloc(pMediaDrvCtx->pDrmBufMgr, "MEDIA", gmmSize, 4096);
                iPitch = gmmPitch;
            }
            else
            {
                bo = mos_bo_alloc_tiled(pMediaDrvCtx->pDrmBufMgr, "MEDIA", gmmPitch, gmmSize/gmmPitch, 1, &tileformat, (unsigned long *)&ulPitch, 0);
                iPitch = (int32_t)ulPitch;
            }
        }
    }
    else
    {
        // Check Pitch and Size
        if (gmmPitch > iPitch || gmmSize > bo->size)
        {
            DDI_ASSERTMESSAGE("External Surface doesn't meet the reqirements of Media driver.");
            DdiMediaUtil_FreeSurface(pMediaSurface);
            hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;

            goto finish;
        }
        
        // Check Alignment
        pGmmResourceInfo->GetRestrictions(gmmRestrictions);
        ulPitchAlign  = gmmRestrictions.RenderPitchAlignment;
        ulSizeAlign   = gmmRestrictions.Alignment;

        if (!MOS_IS_ALIGNED(iPitch, ulPitchAlign) ||
            !MOS_IS_ALIGNED(bo->size, ulSizeAlign))
        {
            DDI_ASSERTMESSAGE("External Surface doesn't meet the reqirements of Media driver.");
            DdiMediaUtil_FreeSurface(pMediaSurface);
            hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;

            goto finish;
        }
    }

    pMediaSurface->bMapped = false;
    if (bo)
    {
#ifdef ANDROID
        if (!bGrallocAllocation)
        {
            intel_ufo_bo_datatype_t datatype;
            
            mos_bo_get_datatype(bo, &datatype.value);
            datatype.is_mmc_capable   = (uint32_t)GmmParams.Flags.Gpu.MMC;
            datatype.compression_hint = INTEL_UFO_BUFFER_HINT_MMC_COMPRESSED;
            mos_bo_set_datatype(bo, datatype.value);
        }
#endif

        pMediaSurface->format      = Format;
        pMediaSurface->iWidth      = iWidth;
        pMediaSurface->iHeight     = gmmHeight;
        pMediaSurface->iRealHeight = iHeight;
        pMediaSurface->iPitch      = iPitch;
        pMediaSurface->iRefCount   = 0;
        pMediaSurface->bo          = bo;
        pMediaSurface->TileType    = tileformat;
        pMediaSurface->isTiled     = (tileformat != I915_TILING_NONE) ? 1 : 0;
        pMediaSurface->pData       = (uint8_t*) bo->virt;
        DDI_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource).",gmmSize, iWidth, iHeight);
    }
    else
    {
        DDI_ASSERTMESSAGE("Fail to Alloc %7d bytes (%d x %d resource).",gmmSize, iWidth, iHeight);
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

finish:
    return hRes;
}

VAStatus DdiMediaUtil_AllocateBuffer(
    DDI_MEDIA_FORMAT            Format,
    int32_t                     iSize,
    PDDI_MEDIA_BUFFER           pMediaBuffer,
    MOS_BUFMGR                 *pBufmgr)
{
    VAStatus                hRes;
    MOS_LINUX_BO           *bo;
    GMM_RESCREATE_PARAMS    GmmParams;

    DDI_CHK_NULL(pMediaBuffer, "pMediaBuffer is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    if(Format >= Media_Format_Count)
       return VA_STATUS_ERROR_INVALID_PARAMETER;

    hRes       = VA_STATUS_SUCCESS;

    bo = mos_bo_alloc(pBufmgr, "Media Buffer", iSize, 4096);

    pMediaBuffer->bMapped = false;
    if (bo)
    {
        pMediaBuffer->format     = Format;
        pMediaBuffer->iSize      = iSize;
        pMediaBuffer->iRefCount  = 0;
        pMediaBuffer->bo         = bo;
        pMediaBuffer->pData      = (uint8_t*) bo->virt;

        DDI_VERBOSEMESSAGE("Alloc %7d bytes resource.",iSize);
    }
    else
    {
        DDI_ASSERTMESSAGE("Fail to Alloc %7d bytes resource.",iSize);
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    // create fake GmmResourceInfo
    MOS_ZeroMemory(&GmmParams, sizeof(GmmParams));
    GmmParams.BaseWidth             = 1;
    GmmParams.BaseHeight            = 1;
    GmmParams.ArraySize             = 0;
    GmmParams.Type                  = RESOURCE_1D;
    GmmParams.Format                = GMM_FORMAT_GENERIC_8BIT;
    GmmParams.Flags.Gpu.Video       = true;
    GmmParams.Flags.Info.Linear     = true;

    pMediaBuffer->pGmmResourceInfo = GmmResCreate(&GmmParams);

    DDI_CHK_NULL(pMediaBuffer->pGmmResourceInfo, "pGmmResourceInfo is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    GmmResOverrideAllocationSize(pMediaBuffer->pGmmResourceInfo, pMediaBuffer->iSize);
    GmmResOverrideAllocationBaseWidth(pMediaBuffer->pGmmResourceInfo, pMediaBuffer->iSize);
    GmmResOverrideAllocationPitch(pMediaBuffer->pGmmResourceInfo, pMediaBuffer->iSize);
finish:
    return hRes;
}

VAStatus DdiMediaUtil_Allocate2DBuffer(
    int32_t                     iHeight,
    int32_t                     iWidth,
    PDDI_MEDIA_BUFFER           pMediaBuffer,
    MOS_BUFMGR                 *pBufmgr)
{
    int32_t                     iSize;
    VAStatus                    hRes;
    MOS_LINUX_BO	           *bo;
    uint32_t                    tileformat;
    uint32_t                    gmmPitch;
    uint32_t                    gmmSize;
    uint32_t                    gmmHeight;
    GMM_RESCREATE_PARAMS        GmmParams;
    GMM_RESOURCE_INFO          *pGmmResourceInfo;

    DDI_CHK_NULL(pMediaBuffer, "pMediaBuffer is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);

    iSize          = 0;
    tileformat     = I915_TILING_NONE;
    hRes           = VA_STATUS_SUCCESS;
    
   
    // Create GmmResourceInfo
    MOS_ZeroMemory(&GmmParams, sizeof(GmmParams));
    GmmParams.BaseWidth             = iWidth;
    GmmParams.BaseHeight            = iHeight;
    GmmParams.ArraySize             = 1;
    GmmParams.Type                  = RESOURCE_2D;
    GmmParams.Format                = GMM_FORMAT_GENERIC_8BIT;

    DDI_CHK_CONDITION(GmmParams.Format == GMM_FORMAT_INVALID, 
                         "Unsupported format", 
                         VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT);

    GmmParams.Flags.Info.Linear = true;
    GmmParams.Flags.Gpu.Video   = true;

    pMediaBuffer->pGmmResourceInfo = pGmmResourceInfo = GmmResCreate(&GmmParams);

    if(nullptr == pGmmResourceInfo)
    {
        DDI_VERBOSEMESSAGE("Gmm Create Resource Failed.");
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    gmmPitch    = (uint32_t)pGmmResourceInfo->GetRenderPitch();
    gmmSize     = GmmResGetRenderSize(pGmmResourceInfo);
    gmmHeight   = pGmmResourceInfo->GetBaseHeight();

   
    bo = mos_bo_alloc(pBufmgr, "Media 2D Buffer", gmmSize, 4096); 

    pMediaBuffer->bMapped = false;
    if (bo)
    {
        pMediaBuffer->format     = Media_Format_2DBuffer;
        pMediaBuffer->iWidth     = iWidth;
        pMediaBuffer->iHeight    = gmmHeight;
        pMediaBuffer->iPitch     = gmmPitch;
        pMediaBuffer->iSize      = gmmSize;
        pMediaBuffer->iRefCount  = 0;
        pMediaBuffer->bo         = bo;
        pMediaBuffer->TileType   = tileformat;
        pMediaBuffer->pData      = (uint8_t*) bo->virt; 
        DDI_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource)\n",iSize, iWidth, iHeight);
    }
    else
    {
        DDI_VERBOSEMESSAGE("Fail to Alloc %7d bytes (%d x %d resource)\n",iSize, iWidth, iHeight);
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

finish:
    return hRes;
}

VAStatus DdiMediaUtil_CreateSurface(DDI_MEDIA_SURFACE  *pSurface, PDDI_MEDIA_CONTEXT pMediaDrvCtx)
{
    VAStatus  hr;

    hr = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(pSurface, "nullptr pSurface", VA_STATUS_ERROR_INVALID_BUFFER);

    // better to differentiate 1D and 2D type
    hr = DdiMediaUtil_AllocateSurface(pSurface->format, 
                         pSurface->iWidth, 
                         pSurface->iHeight, 
                         pSurface, 
                         pMediaDrvCtx);
    if (VA_STATUS_SUCCESS == hr && nullptr != pSurface->bo)
        pSurface->base = pSurface->name;

    return hr;
}

VAStatus DdiMediaUtil_CreateBuffer(DDI_MEDIA_BUFFER *pBuffer, MOS_BUFMGR *pBufmgr)
{
    VAStatus  hr;

    hr = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(pBuffer, "nullptr pBuffer", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_CHK_LESS(pBuffer->format, Media_Format_Count, "Invalid pBuffer->format", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (pBuffer->format == Media_Format_CPU)
    {
        pBuffer->pData= (uint8_t*)MOS_AllocAndZeroMemory(pBuffer->iSize);
        if (nullptr == pBuffer->pData)
            hr = VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    else
    {
        if (Media_Format_2DBuffer == pBuffer->format)
        {
            hr = DdiMediaUtil_Allocate2DBuffer(pBuffer->iHeight,
                                  pBuffer->iWidth,
                                  pBuffer,
                                  pBufmgr);
         }
         else
         {
             hr = DdiMediaUtil_AllocateBuffer(pBuffer->format, 
                                 pBuffer->iSize,
                                 pBuffer,
                                 pBufmgr);
         }
    }

    pBuffer->uiLockedBufID   = VA_INVALID_ID;
    pBuffer->uiLockedImageID = VA_INVALID_ID;
    pBuffer->iRefCount       = 0;


    return hr;
}

// add thread protection for multiple thread?
void* DdiMediaUtil_LockSurface(DDI_MEDIA_SURFACE  *pSurface, uint32_t flag)
{
    if(DDI_UTIL_CHK_NULL(pSurface))
        return nullptr;

    if(DDI_UTIL_CHK_NULL(pSurface->bo))
        return nullptr;

    if((false == pSurface->bMapped) && (0 == pSurface->iRefCount))
    {
        if (pSurface->pMediaCtx->bIsAtomSOC)
        {
#ifdef ANDROID
            if (pSurface->iWidth * pSurface->iHeight * 3 <= GTT_SIZE_THRESHOLD)
            {
                mos_gem_bo_map_gtt(pSurface->bo);
            }
            else
            {
                mos_bo_map(pSurface->bo, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));
                if (pSurface->TileType != I915_TILING_NONE)
                {
                    if (NeedSwizzleData(pSurface, true) == false)
                        return nullptr;
                }
            }
#else
            mos_gem_bo_map_gtt(pSurface->bo);
#endif
        }
        else
        {
            if (pSurface->TileType == I915_TILING_NONE)
            {
                mos_bo_map(pSurface->bo, flag & MOS_LOCKFLAG_WRITEONLY);
            }
            else if (flag & MOS_LOCKFLAG_WRITEONLY)
            {
                mos_gem_bo_map_gtt(pSurface->bo);
            }
            else 
            {
                mos_gem_bo_map_unsynchronized(pSurface->bo);     // only call mmap_gtt ioctl
                mos_gem_bo_start_gtt_access(pSurface->bo, 0);    // set to GTT domain,0 means readonly
            }
        }
        pSurface->pData   = (uint8_t*) pSurface->bo->virt;
        pSurface->bMapped = true;
    }
    else
    {
        // do nothing here
    }
    pSurface->iRefCount++;

    return pSurface->pData;
}

void DdiMediaUtil_UnlockSurface(DDI_MEDIA_SURFACE  *pSurface)
{
    if(DDI_UTIL_CHK_NULL(pSurface))
        return;

    if(DDI_UTIL_CHK_NULL(pSurface->bo))
        return;

    if (0 == pSurface->iRefCount)
        return;

    if((true == pSurface->bMapped) && (1 == pSurface->iRefCount))
    {
        if (pSurface->pMediaCtx->bIsAtomSOC)
        {
#ifdef ANDROID
            if (pSurface->iWidth * pSurface->iHeight * 3 <= GTT_SIZE_THRESHOLD)
            {
                mos_gem_bo_unmap_gtt(pSurface->bo);
            }
            else
            {
                if (pSurface->TileType != I915_TILING_NONE)
                    NeedSwizzleData(pSurface, false);

                mos_bo_unmap(pSurface->bo);
            }
#else
            mos_gem_bo_unmap_gtt(pSurface->bo);
#endif
        }
        else
        {
            if (pSurface->TileType == I915_TILING_NONE)
            {
               mos_bo_unmap(pSurface->bo);
            }
            else
            {
               mos_gem_bo_unmap_gtt(pSurface->bo);
            }
        }
        pSurface->pData       = nullptr;
        pSurface->bo->virt    = nullptr;
        pSurface->bMapped     = false;
    }
    else
    {
        // do nothing here
    }

    pSurface->iRefCount--;

    return;
}

// add thread protection for multiple thread?
// MapBuffer?
void* DdiMediaUtil_LockBuffer(DDI_MEDIA_BUFFER *pBuf, uint32_t flag)
{
    if(DDI_UTIL_CHK_NULL(pBuf))
        return nullptr;

    if((Media_Format_CPU != pBuf->format) && (false == pBuf->bMapped))
    {
        if (nullptr != pBuf->pSurface)
        {
            DdiMediaUtil_LockSurface(pBuf->pSurface, flag);
            pBuf->pData = pBuf->pSurface->pData;
        }
        else
        {
            if (pBuf->pMediaCtx->bIsAtomSOC)
            {
                mos_gem_bo_map_gtt(pBuf->bo);
            }
            else
            {
                if (pBuf->TileType == I915_TILING_NONE)
                {
                    mos_bo_map(pBuf->bo, ((MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY) & flag));
                }
                else
                {
                    mos_gem_bo_map_gtt(pBuf->bo);
                }
             }

            pBuf->pData = (uint8_t*)(pBuf->bo->virt);
        }

        pBuf->bMapped = true;
        pBuf->iRefCount++;
    }
    else if ((Media_Format_CPU == pBuf->format) && (false == pBuf->bMapped))
    {
        pBuf->bMapped = true;
        pBuf->iRefCount++;
    }
    else 
    {
        pBuf->iRefCount++;
    }

    return pBuf->pData;
}

void DdiMediaUtil_UnlockBuffer(DDI_MEDIA_BUFFER *pBuf)
{
    if(DDI_UTIL_CHK_NULL(pBuf))
        return;

    if (0 == pBuf->iRefCount)
        return;
    if((true == pBuf->bMapped) && (Media_Format_CPU != pBuf->format) && (1 == pBuf->iRefCount))
    {
        if (nullptr != pBuf->pSurface)
        {
            DdiMediaUtil_UnlockSurface(pBuf->pSurface);
        }
        else
        {
             if (pBuf->pMediaCtx->bIsAtomSOC)
             {
                 mos_gem_bo_unmap_gtt(pBuf->bo);
             }
             else
             {
                 if (pBuf->TileType == I915_TILING_NONE)
                 {
                     mos_bo_unmap(pBuf->bo);
                 }
                 else
                 {
                     mos_gem_bo_unmap_gtt(pBuf->bo);
                 }
            }
            pBuf->bo->virt = nullptr;
        }

        pBuf->pData       = nullptr;

        pBuf->bMapped     = false;
    }
    else if ((true == pBuf->bMapped) && (Media_Format_CPU == pBuf->format) && (1 == pBuf->iRefCount))
    {
        pBuf->bMapped     = false;
    }
    else 
    {
        // do nothing here
    }
    pBuf->iRefCount--;
    return;
}

// should ref_count added for bo?
void DdiMediaUtil_FreeSurface(DDI_MEDIA_SURFACE *pSurface)
{ 
    if(DDI_UTIL_CHK_NULL(pSurface))
        return;

    if(DDI_UTIL_CHK_NULL(pSurface->bo))
        return;

    // For External Buffer, only needs to destory SurfaceDescriptor
    if ( DdiMediaUtil_IsExternalSurface(pSurface) )
    {
        // In DdiMediaUtil_AllocateSurface call, driver will increase the surface reference count by calling drm_intel_bo_gem_create_from_name
        // Thus, when freeing the surface, the drm_intel_bo_unreference function should be called to avoid memory leak
        mos_bo_unreference(pSurface->bo);
        MOS_FreeMemory(pSurface->pSurfDesc);
        pSurface->pSurfDesc = nullptr;
    }
    else
    {
        // calling sequence checking
        if (pSurface->bMapped)
        {
            DdiMediaUtil_UnlockSurface(pSurface);
            DDI_VERBOSEMESSAGE("DDI: try to free a locked surface.");
        }
        mos_bo_unreference(pSurface->bo);
        pSurface->bo = nullptr;
    }

    if (nullptr != pSurface->pGmmResourceInfo)
    {
        GmmResFree(pSurface->pGmmResourceInfo);
        pSurface->pGmmResourceInfo = nullptr;
    }
}


// should ref_count added for bo?
void DdiMediaUtil_FreeBuffer(DDI_MEDIA_BUFFER  *pBuf)
{
    if(DDI_UTIL_CHK_NULL(pBuf))
        return;

    // calling sequence checking
    if (pBuf->bMapped)
    {
        DdiMediaUtil_UnlockBuffer(pBuf);
        DDI_VERBOSEMESSAGE("DDI: try to free a locked buffer.");
    }
    if (pBuf->format == Media_Format_CPU)
    {
        MOS_FreeMemory(pBuf->pData);
        pBuf->pData = nullptr;
    }
    else
    {
        mos_bo_unreference(pBuf->bo);
        pBuf->bo = nullptr;
    }

    if (nullptr != pBuf->pGmmResourceInfo)
    {
        GmmResFree(pBuf->pGmmResourceInfo);
        pBuf->pGmmResourceInfo = nullptr;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// Purpose:      combine a DDI_MEDIA_BUFFER and a VaImage to a DDI_MEDIA_SURFACE structure
// pBuf[in]:     pointer to input buffer
// pImage[in]:   pointer to input image
// pSurface[in]: pointer to combined surface
/////////////////////////////////////////////////////////////////////////////////////
VAStatus DdiMediaUtil_ConvertBufImageToSurface(DDI_MEDIA_BUFFER *pBuf, VAImage *pImage, DDI_MEDIA_SURFACE *pSurface)
{
    DDI_CHK_NULL(pBuf,    "Invalid buffer.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pImage,  "Invalid image.",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pSurface,"Invalid surface.",VA_STATUS_ERROR_INVALID_PARAMETER);

    pSurface->bo              = pBuf->bo;
    pSurface->bMapped         = pBuf->bMapped;
    pSurface->format          = pBuf->format;
    pSurface->iRefCount       = pBuf->iRefCount;
    pSurface->iHeight         = pImage->height;
    pSurface->iWidth          = pImage->width;
    pSurface->iPitch          = pImage->pitches[0];
    pSurface->TileType        = pBuf->TileType;
    pSurface->uiLockedBufID   = pBuf->uiLockedBufID;
    pSurface->uiLockedImageID = pBuf->uiLockedImageID;
    pSurface->pData           = pBuf->pData;
    pSurface->uiOffset        = 0;
    pSurface->name            = pBuf->name;
    pSurface->base            = pSurface->name;

    return VA_STATUS_SUCCESS;
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

void DdiMediaUtil_InitMutex(PMEDIA_MUTEX_T  pMutex)
{
    pthread_mutex_init(pMutex, nullptr);
}
void DdiMediaUtil_DestroyMutex(PMEDIA_MUTEX_T  pMutex)
{
    int32_t ret = 0;
    ret = pthread_mutex_destroy(pMutex);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't destroy the mutex!\n");
    }
}
void DdiMediaUtil_LockMutex(PMEDIA_MUTEX_T  pMutex)
{
    int32_t ret = 0;
    ret = pthread_mutex_lock(pMutex);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't lock the mutex!\n");
    }
}
void DdiMediaUtil_UnLockMutex(PMEDIA_MUTEX_T  pMutex)
{
    int32_t ret = 0;
    ret = pthread_mutex_unlock(pMutex);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't unlock the mutex!\n");
    }
}

void DdiMediaUtil_InitSemaphore(PMEDIA_SEM_T  pSem, uint32_t uiInitCount)
{
    int32_t ret = 0;
    ret = sem_init(pSem, 0, uiInitCount);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't initialize the semaphore!\n");
    }
}

void DdiMediaUtil_DestroySemaphore(PMEDIA_SEM_T  pSem)
{
    int32_t ret = 0;
    ret = sem_destroy(pSem);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't destroy the semaphore!\n");
    }
}
void DdiMediaUtil_WaitSemaphore(PMEDIA_SEM_T  pSem)
{
    int32_t ret = 0;
    ret = sem_wait(pSem);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("wait semaphore error!\n");
    }
}

int32_t DdiMediaUtil_TryWaitSemaphore(PMEDIA_SEM_T  pSem)
{
    return sem_trywait(pSem);
}

void DdiMediaUtil_PostSemaphore(PMEDIA_SEM_T  pSem)
{
    int32_t ret = 0;
    ret = sem_post(pSem);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("post semaphore error!\n");
    }
}

// heap related
PDDI_MEDIA_SURFACE_HEAP_ELEMENT DdiMediaUtil_AllocPMediaSurfaceFromHeap(PDDI_MEDIA_HEAP pSurfaceHeap)
{
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  pSurfaceHeapBase;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  pMediaSurfaceHeapElmt;
    void                            *pNewHeapBase;
    int32_t                          i;

    if (nullptr == pSurfaceHeap->pFirstFreeHeapElement)
    {
        pNewHeapBase = realloc(pSurfaceHeap->pHeapBase, (pSurfaceHeap->uiAllocatedHeapElements + DDI_MEDIA_HEAP_INCREMENTAL_SIZE) * sizeof(DDI_MEDIA_SURFACE_HEAP_ELEMENT));

        if (nullptr == pNewHeapBase)
        {
            DDI_ASSERTMESSAGE("DDI: realloc failed.");
            return nullptr;
        }
        pSurfaceHeap->pHeapBase                    = pNewHeapBase;
        pSurfaceHeapBase                           = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)pSurfaceHeap->pHeapBase;
        pSurfaceHeap->pFirstFreeHeapElement        = (void*)(&pSurfaceHeapBase[pSurfaceHeap->uiAllocatedHeapElements]);
        for (i = 0; i < (DDI_MEDIA_HEAP_INCREMENTAL_SIZE); i++)
        {
            pMediaSurfaceHeapElmt                  = &pSurfaceHeapBase[pSurfaceHeap->uiAllocatedHeapElements + i];
            pMediaSurfaceHeapElmt->pNextFree       = (i == (DDI_MEDIA_HEAP_INCREMENTAL_SIZE - 1))? nullptr : &pSurfaceHeapBase[pSurfaceHeap->uiAllocatedHeapElements + i + 1];
            pMediaSurfaceHeapElmt->uiVaSurfaceID   = pSurfaceHeap->uiAllocatedHeapElements + i;
        }
        pSurfaceHeap->uiAllocatedHeapElements     += DDI_MEDIA_HEAP_INCREMENTAL_SIZE;
    }

    pMediaSurfaceHeapElmt                          = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)pSurfaceHeap->pFirstFreeHeapElement;
    pSurfaceHeap->pFirstFreeHeapElement            = pMediaSurfaceHeapElmt->pNextFree;

    return pMediaSurfaceHeapElmt;
}


void DdiMediaUtil_ReleasePMediaSurfaceFromHeap(PDDI_MEDIA_HEAP pSurfaceHeap, uint32_t uiVaSurfaceID)
{
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  pMediaSurfaceHeapElmt;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  pMediaSurfaceHeapBase;
    void                            *pFirstFree;

    DDI_CHK_LESS(uiVaSurfaceID, pSurfaceHeap->uiAllocatedHeapElements, "invalid surface id", );
    pMediaSurfaceHeapBase                   = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)pSurfaceHeap->pHeapBase;
    pMediaSurfaceHeapElmt                   = &pMediaSurfaceHeapBase[uiVaSurfaceID];
    DDI_CHK_NULL(pMediaSurfaceHeapElmt->pSurface, "surface is already released", );
    pFirstFree                              = pSurfaceHeap->pFirstFreeHeapElement;
    pSurfaceHeap->pFirstFreeHeapElement     = (void*)pMediaSurfaceHeapElmt;
    pMediaSurfaceHeapElmt->pNextFree        = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)pFirstFree;
    pMediaSurfaceHeapElmt->pSurface         = nullptr;
}


PDDI_MEDIA_BUFFER_HEAP_ELEMENT DdiMediaUtil_AllocPMediaBufferFromHeap(PDDI_MEDIA_HEAP pBufferHeap)
{
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT  pMediaBufferHeapBase;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT  pMediaBufferHeapElmt;
    void                           *pNewHeapBase;
    int32_t                         i;

    if (nullptr == pBufferHeap->pFirstFreeHeapElement)
    {
        pNewHeapBase = realloc(pBufferHeap->pHeapBase, (pBufferHeap->uiAllocatedHeapElements + DDI_MEDIA_HEAP_INCREMENTAL_SIZE) * sizeof(DDI_MEDIA_BUFFER_HEAP_ELEMENT));
        if (nullptr == pNewHeapBase)
        {
            DDI_ASSERTMESSAGE("DDI: realloc failed.");
            return nullptr;
        }
        pBufferHeap->pHeapBase                 = pNewHeapBase;
        pMediaBufferHeapBase                   = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)pBufferHeap->pHeapBase;
        pBufferHeap->pFirstFreeHeapElement     = (void*)(&pMediaBufferHeapBase[pBufferHeap->uiAllocatedHeapElements]);
        for (i = 0; i < (DDI_MEDIA_HEAP_INCREMENTAL_SIZE); i++)
        {
            pMediaBufferHeapElmt               = &pMediaBufferHeapBase[pBufferHeap->uiAllocatedHeapElements + i];
            pMediaBufferHeapElmt->pNextFree    = (i == (DDI_MEDIA_HEAP_INCREMENTAL_SIZE - 1))? nullptr : &pMediaBufferHeapBase[pBufferHeap->uiAllocatedHeapElements + i + 1];
            pMediaBufferHeapElmt->uiVaBufferID = pBufferHeap->uiAllocatedHeapElements + i;
        }
        pBufferHeap->uiAllocatedHeapElements  += DDI_MEDIA_HEAP_INCREMENTAL_SIZE;
    }

    pMediaBufferHeapElmt                       = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)pBufferHeap->pFirstFreeHeapElement;
    pBufferHeap->pFirstFreeHeapElement         = pMediaBufferHeapElmt->pNextFree;
    return pMediaBufferHeapElmt;
}


void DdiMediaUtil_ReleasePMediaBufferFromHeap(PDDI_MEDIA_HEAP pBufferHeap, uint32_t uiVaBufferID)
{
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT   pMediaBufferHeapBase;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT   pMediaBufferHeapElmt;
    void                            *pFirstFree;

    DDI_CHK_LESS(uiVaBufferID, pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", );
    pMediaBufferHeapBase                    = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)pBufferHeap->pHeapBase;
    pMediaBufferHeapElmt                    = &pMediaBufferHeapBase[uiVaBufferID];
    DDI_CHK_NULL(pMediaBufferHeapElmt->pBuffer, "buffer is already released", );
    pFirstFree                              = pBufferHeap->pFirstFreeHeapElement;
    pBufferHeap->pFirstFreeHeapElement      = (void*)pMediaBufferHeapElmt;
    pMediaBufferHeapElmt->pNextFree         = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)pFirstFree;
    pMediaBufferHeapElmt->pBuffer           = nullptr;
}

PDDI_MEDIA_IMAGE_HEAP_ELEMENT DdiMediaUtil_AllocPVAImageFromHeap(PDDI_MEDIA_HEAP pImageHeap)
{
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT   pVAImageHeapBase;
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT   pVAImageHeapElmt;
    void                           *pNewHeapBase;
    int32_t                         i;

    if (nullptr == pImageHeap->pFirstFreeHeapElement)
    {
        pNewHeapBase = realloc(pImageHeap->pHeapBase, (pImageHeap->uiAllocatedHeapElements + DDI_MEDIA_HEAP_INCREMENTAL_SIZE) * sizeof(DDI_MEDIA_IMAGE_HEAP_ELEMENT));

        if (nullptr == pNewHeapBase)
        {
            DDI_ASSERTMESSAGE("DDI: realloc failed.");
            return nullptr;
        }
        pImageHeap->pHeapBase                  = pNewHeapBase;
        pVAImageHeapBase                       = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)pImageHeap->pHeapBase;
        pImageHeap->pFirstFreeHeapElement      = (void*)(&pVAImageHeapBase[pImageHeap->uiAllocatedHeapElements]);
        for (i = 0; i < (DDI_MEDIA_HEAP_INCREMENTAL_SIZE); i++)
        {
            pVAImageHeapElmt                   = &pVAImageHeapBase[pImageHeap->uiAllocatedHeapElements + i];
            pVAImageHeapElmt->pNextFree        = (i == (DDI_MEDIA_HEAP_INCREMENTAL_SIZE - 1))? nullptr : &pVAImageHeapBase[pImageHeap->uiAllocatedHeapElements + i + 1];
            pVAImageHeapElmt->uiVaImageID      = pImageHeap->uiAllocatedHeapElements + i;
        }
        pImageHeap->uiAllocatedHeapElements   += DDI_MEDIA_HEAP_INCREMENTAL_SIZE;

    }

    pVAImageHeapElmt                           = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)pImageHeap->pFirstFreeHeapElement;
    pImageHeap->pFirstFreeHeapElement          = pVAImageHeapElmt->pNextFree;
    return pVAImageHeapElmt;
}


void DdiMediaUtil_ReleasePVAImageFromHeap(PDDI_MEDIA_HEAP pImageHeap, uint32_t uiVAImageID)
{
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT    pVAImageHeapBase;
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT    pVAImageHeapElmt;
    void                            *pFirstFree;

    DDI_CHK_LESS(uiVAImageID, pImageHeap->uiAllocatedHeapElements, "invalid image id", );
    pVAImageHeapBase                    = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)pImageHeap->pHeapBase;
    pVAImageHeapElmt                    = &pVAImageHeapBase[uiVAImageID];
    DDI_CHK_NULL(pVAImageHeapElmt->pImage, "image is already released", );
    pFirstFree                          = pImageHeap->pFirstFreeHeapElement;
    pImageHeap->pFirstFreeHeapElement   = (void*)pVAImageHeapElmt;
    pVAImageHeapElmt->pNextFree         = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)pFirstFree;
    pVAImageHeapElmt->pImage            = nullptr;
}

PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT DdiMediaUtil_AllocPVAContextFromHeap(PDDI_MEDIA_HEAP pVaContextHeap)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT   pVAContextHeapBase;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT   pVAContextHeapElmt;
    void                               *pNewHeapBase;
    int32_t                                 i;

    if (nullptr == pVaContextHeap->pFirstFreeHeapElement)
    {
        pNewHeapBase = realloc(pVaContextHeap->pHeapBase, (pVaContextHeap->uiAllocatedHeapElements + DDI_MEDIA_HEAP_INCREMENTAL_SIZE) * sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT));

        if (nullptr == pNewHeapBase)
        {
            DDI_ASSERTMESSAGE("DDI: realloc failed.");
            return nullptr;
        }
        pVaContextHeap->pHeapBase                    = pNewHeapBase;
        pVAContextHeapBase                           = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)pVaContextHeap->pHeapBase;
        pVaContextHeap->pFirstFreeHeapElement        = (void*)(&(pVAContextHeapBase[pVaContextHeap->uiAllocatedHeapElements]));
        for (i = 0; i < (DDI_MEDIA_HEAP_INCREMENTAL_SIZE); i++)
        {
            pVAContextHeapElmt                       = &pVAContextHeapBase[pVaContextHeap->uiAllocatedHeapElements + i];
            pVAContextHeapElmt->pNextFree            = (i == (DDI_MEDIA_HEAP_INCREMENTAL_SIZE - 1))? nullptr : &pVAContextHeapBase[pVaContextHeap->uiAllocatedHeapElements + i + 1];
            pVAContextHeapElmt->uiVaContextID        = pVaContextHeap->uiAllocatedHeapElements + i;
            pVAContextHeapElmt->pVaContext           = nullptr;
        }
        pVaContextHeap->uiAllocatedHeapElements     += DDI_MEDIA_HEAP_INCREMENTAL_SIZE;
    }

    pVAContextHeapElmt                               = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)pVaContextHeap->pFirstFreeHeapElement;
    pVaContextHeap->pFirstFreeHeapElement            = pVAContextHeapElmt->pNextFree;
    return pVAContextHeapElmt;
}


void DdiMediaUtil_ReleasePVAContextFromHeap(PDDI_MEDIA_HEAP pVaContextHeap, uint32_t uiVAContextID)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT    pVAContextHeapElmt;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT    pVAContextHeapBase;
    void                                *pFirstFree;

    DDI_CHK_LESS(uiVAContextID, pVaContextHeap->uiAllocatedHeapElements, "invalid context id", );
    pVAContextHeapBase                      = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)pVaContextHeap->pHeapBase;
    pVAContextHeapElmt                      = &pVAContextHeapBase[uiVAContextID];
    DDI_CHK_NULL(pVAContextHeapElmt->pVaContext, "context is already released", );
    pFirstFree                              = pVaContextHeap->pFirstFreeHeapElement;
    pVaContextHeap->pFirstFreeHeapElement   = (void*)pVAContextHeapElmt;
    pVAContextHeapElmt->pNextFree           = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)pFirstFree;
    pVAContextHeapElmt->pVaContext          = nullptr;
}

void DdiMediaUtil_UnRefBufObjInMediaBuffer(PDDI_MEDIA_BUFFER pBuf)
{
    mos_bo_unreference(pBuf->bo);
}

void DdiMediaUtil_GetEnabledFeature(PDDI_MEDIA_CONTEXT pMediaCtx)
{
    MOS_USER_FEATURE       UserFeature;
    MOS_USER_FEATURE_VALUE UserFeatureValue;

    DDI_CHK_NULL(pMediaCtx, "Pointer is nullptr", );
    // Reads and stores the user feature which enables binary instead of text dumps
    MOS_ZeroMemory(&UserFeatureValue, sizeof(UserFeatureValue));
    UserFeatureValue.u32Data    = true;
    UserFeature.Type            = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pValues         = &UserFeatureValue;
    UserFeature.uiNumValues     = 1;
    MOS_UserFeature_ReadValue(
        nullptr,
        &UserFeature,
        "VC1Enabled",
        MOS_USER_FEATURE_VALUE_TYPE_INT32);

    pMediaCtx->bVC1Enabled = (UserFeatureValue.u32Data == 1) ? true : false;

#ifdef ANDROID
    pMediaCtx->bVC1Enabled = false;
#endif
}

// Open Intel's Graphics Device to get the file descriptor
int32_t DdiMediaUtil_OpenGraphicsAdaptor(char *pDevName)
{
    struct stat st;
    int32_t         hDevice;

    hDevice = -1;

    if(nullptr == pDevName)
    {
        DDI_ASSERTMESSAGE("Invalid Graphics Node");
        return -1;
    }

    if (-1 == stat (pDevName, &st))
    {
        DDI_ASSERTMESSAGE("Cannot identify '%s': %d, %s.", pDevName, errno, strerror (errno));
        return -1;
    }

    if (!S_ISCHR (st.st_mode))
    {
        DDI_ASSERTMESSAGE("%s is no device.", pDevName);
        return -1;
    }

    hDevice = open (pDevName, O_RDWR);
    if (-1 == hDevice)
    {
        DDI_ASSERTMESSAGE("Cannot open '%s': %d, %s.", pDevName, errno, strerror (errno));
        return -1;
    }

    return hDevice;
}
