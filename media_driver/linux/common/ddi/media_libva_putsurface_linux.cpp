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
//! \file      media_libva_putsurface_linux.cpp
//! \brief     libva(and its extension) putsurface linux implementaion
//!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <fcntl.h>     //open
#include <sys/stat.h>  //fstat
#include <unistd.h>    //read, lseek
#include <dlfcn.h>     //dlopen,dlsym,dlclose
#include <time.h>      //get_clocktime
#include <errno.h>     //errno
#include <assert.h>    //assert

#include <sys/mman.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "media_libva_putsurface_linux.h"
#include "media_libva_util.h"
#include "media_libva_common.h"
#include "media_libva_vp.h"

extern MOS_FORMAT     VpGetFormatFromMediaFormat(DDI_MEDIA_FORMAT mf);
extern VPHAL_CSPACE   DdiVp_GetColorSpaceFromMediaFormat(DDI_MEDIA_FORMAT mf);
extern MOS_TILE_TYPE  VpGetTileTypeFromMediaTileType(uint32_t mediaTileType);

/* Closes and disposed any allocated data */
void dso_close(struct dso_handle *h)
{
    if (!h){
        return;
    }

    if (h->handle) {
        if (h->handle != RTLD_DEFAULT)
            dlclose(h->handle);
        h->handle = nullptr;
    }
    free(h);
}

/* Opens the named shared library */
struct dso_handle * dso_open(const char *path)
{
    struct dso_handle *h = nullptr;

    h = (dso_handle *)calloc(1, sizeof(*h));
    if (!h){
        return nullptr;
    }

    if (path) {
        h->handle = dlopen(path, RTLD_LAZY|RTLD_LOCAL);
        if (!h->handle)
            goto error;
    }
    else{
        h->handle = RTLD_DEFAULT;
    }
    return h;

error:
    dso_close(h);
    return nullptr;
}

/* Load function name from one dynamic lib */
static bool get_symbol(struct dso_handle *h, void *func_vptr, const char *name)
{
    DDI_CHK_NULL(h, "nullptr h", false);
    DDI_CHK_NULL(func_vptr, "nullptr func_vptr", false);

    dso_generic_func func;
    dso_generic_func * const func_ptr = (dso_generic_func*) func_vptr;
    const char *error = nullptr;

    dlerror();
    func = (dso_generic_func)dlsym(h->handle, name);
    error = dlerror();
    if (error) {
        fprintf(stderr, "error: failed to resolve %s(): %s\n", name, error);
        return false;
    }
    *func_ptr = func;
    return true;
}

//!
//! \brief  Loads function name from vtable
//!
//! \param  [in] h
//!     Dso handle
//! \param  [in] vtable
//!     VA api table
//! \param  [in] vtable_length
//!     Length of VA api table
//! \param  [in] symbols
//!     Dso symbol
//!
//! \return     bool 
//!     true if call success, else false
//!
bool
dso_get_symbols(
    struct dso_handle          *h,
    void                       *vtable,
    uint32_t                    vtable_length,
    const struct dso_symbol    *symbols
)
{
    DDI_CHK_NULL(h, "nullptr h", false);

    const struct dso_symbol *s = nullptr;
    if (nullptr == symbols)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    for (s = symbols; s->name != nullptr; s++) {
        if (s->offset + sizeof(dso_generic_func) > vtable_length)
            return false;
        if (!get_symbol(h, ((char *)vtable) + s->offset, s->name))
            return false;
    }
    return true;
}

bool output_dri_init(VADriverContextP ctx)
{
    DDI_CHK_NULL(ctx, "nullptr ctx", false);

    PDDI_MEDIA_CONTEXT mediaDrvCtx = nullptr;
    mediaDrvCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx, "nullptr ctx", false);

    struct dso_handle *dso_handle = nullptr;
    struct dri_vtable *dri_vtable = nullptr;

    mediaDrvCtx->dri_output = nullptr;

    static const struct dso_symbol symbols[] = {
        { "va_dri_get_drawable",
          offsetof(struct dri_vtable, get_drawable) },
        { "va_dri_get_rendering_buffer",
          offsetof(struct dri_vtable, get_rendering_buffer) },
        { "va_dri_swap_buffer",
          offsetof(struct dri_vtable, swap_buffer) },
        { nullptr, }
    };

    mediaDrvCtx->dri_output = (va_dri_output*) calloc(1, sizeof(struct va_dri_output));
    if (!mediaDrvCtx->dri_output){
        goto error;
    }

    mediaDrvCtx->dri_output->handle = dso_open(LIBVA_X11_NAME);
    if (!mediaDrvCtx->dri_output->handle){
        free(mediaDrvCtx->dri_output);
        mediaDrvCtx->dri_output = nullptr;
        goto error;
    }

    dso_handle = mediaDrvCtx->dri_output->handle;
    dri_vtable = &mediaDrvCtx->dri_output->vtable;
    if (!dso_get_symbols(dso_handle, dri_vtable, sizeof(*dri_vtable), symbols)){
        dso_close(mediaDrvCtx->dri_output->handle);
        free(mediaDrvCtx->dri_output);
        mediaDrvCtx->dri_output = nullptr;
        goto error;
    }
    return true;

error:
    return false;
}

void
inline Rect_init(
    RECT            *rect,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth
)
{
    if (nullptr == rect)
    {
        return;
    }
    rect->left                    = destx;
    rect->top                     = desty;
    rect->right                   = destx + destw;
    rect->bottom                  = desty + desth;
}

VAStatus DdiCodec_PutSurfaceLinuxVphalExt(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void            *draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,       /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
)
{
    GC                        gc;
    int32_t                   depth;
    Visual*                   visual;
    XImage*                   ximg;
    int32_t                   surf_width;
    int32_t                   surf_height;
    PDDI_MEDIA_CONTEXT        mediaDrvCtx;
    PDDI_MEDIA_SURFACE        dstSurfBuffObj;

    TypeXCreateGC             pfn_XCreateGC = nullptr;
    TypeXFreeGC               pfn_XFreeGC = nullptr;
    TypeXCreateImage          pfn_XCreateImage = nullptr;
    TypeXDestroyImage         pfn_XDestroyImage = nullptr;
    TypeXPutImage             pfn_XPutImage = nullptr;

    if (nullptr == draw)
    {
        return VA_STATUS_ERROR_UNKNOWN;
    }

    visual                    = nullptr;
    ximg                     = nullptr;
    mediaDrvCtx              = DdiMedia_GetMediaContext(ctx);
    dstSurfBuffObj           = DdiMedia_GetSurfaceFromVASurfaceID(mediaDrvCtx, surface);

    if (nullptr == dstSurfBuffObj)
    {
        return VA_STATUS_ERROR_UNKNOWN;
    }

    if (nullptr == mediaDrvCtx->X11FuncTable                   ||
        nullptr == mediaDrvCtx->X11FuncTable->pfnXCreateGC     ||
        nullptr == mediaDrvCtx->X11FuncTable->pfnXFreeGC       ||
        nullptr == mediaDrvCtx->X11FuncTable->pfnXCreateImage  ||
        nullptr == mediaDrvCtx->X11FuncTable->pfnXDestroyImage ||
        nullptr == mediaDrvCtx->X11FuncTable->pfnXPutImage)
    {
        return VA_STATUS_ERROR_UNKNOWN;
    }

    pfn_XCreateGC     = (TypeXCreateGC)(mediaDrvCtx->X11FuncTable->pfnXCreateGC);
    pfn_XFreeGC       = (TypeXFreeGC)(mediaDrvCtx->X11FuncTable->pfnXFreeGC);
    pfn_XCreateImage  = (TypeXCreateImage)(mediaDrvCtx->X11FuncTable->pfnXCreateImage);
    pfn_XDestroyImage = (TypeXDestroyImage)(mediaDrvCtx->X11FuncTable->pfnXDestroyImage);
    pfn_XPutImage     = (TypeXPutImage)(mediaDrvCtx->X11FuncTable->pfnXPutImage);

    surf_width  = dstSurfBuffObj->iWidth;
    surf_height = dstSurfBuffObj->iHeight;

    visual = DefaultVisual(ctx->native_dpy, ctx->x11_screen);
    gc     = (*pfn_XCreateGC)((Display*)ctx->native_dpy, (Drawable)draw, 0, nullptr);
    depth  = DefaultDepth(ctx->native_dpy, ctx->x11_screen);

    if (TrueColor != visual->c_class)
    {
        DDI_ASSERTMESSAGE("Default visual of X display must be TrueColor.");
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    ximg = (*pfn_XCreateImage)((Display*)ctx->native_dpy, visual, depth, ZPixmap, 0, nullptr,surf_width, surf_height, 32, 0 );

    if (nullptr == ximg)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (ximg->bits_per_pixel != 32)
    {
        DDI_ASSERTMESSAGE("Display uses %d bits/pixel this not supported.",ximg->bits_per_pixel);
        (*pfn_XDestroyImage)(ximg);
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    ximg->data = (char *)DdiMediaUtil_LockSurface(dstSurfBuffObj, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));

    if (nullptr == ximg->data)
    {
        DdiMediaUtil_UnlockSurface(dstSurfBuffObj);
        (*pfn_XDestroyImage)(ximg);
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    (*pfn_XPutImage)((Display*)ctx->native_dpy, (Drawable)draw, gc, ximg, 0, 0, destx, desty, surf_width, surf_height);

    DdiMediaUtil_UnlockSurface(dstSurfBuffObj);
    ximg->data = nullptr;

    if (nullptr != ximg)
    {
        (*pfn_XDestroyImage)(ximg);
    }

    (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiCodec_PutSurfaceLinuxHW(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void*            draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
)
{
    VpBase                  *vpHal = nullptr;
    int32_t                 ovRenderIndex = 0;
    VPHAL_SURFACE           Surf;
    VPHAL_SURFACE           target;
    VPHAL_RENDER_PARAMS     renderParams;
    VPHAL_COLORFILL_PARAMS  colorFill;

    MOS_STATUS              eStatus    = MOS_STATUS_INVALID_PARAMETER;
    RECT                    srcRect    = { 0, 0, 0, 0 };
    RECT                    dstRect    = { 0, 0, 0, 0 };
    PDDI_MEDIA_CONTEXT      mediaCtx;
    PDDI_MEDIA_SURFACE      bufferObject;
    uint32_t                width,height,pitch;
    uint32_t                drawable_tiling_mode;
    uint32_t                drawable_swizzle_mode;
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_TILE_TYPE           tileType;

    uint32_t                ctxType;
    PDDI_VP_CONTEXT         vpCtx;
    struct dri_drawable*    dri_drawable = nullptr;
    union dri_buffer*       buffer = nullptr;

    GMM_RESCREATE_PARAMS    gmmParams;

    mediaCtx     = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "Null mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->dri_output, "Null mediaDrvCtx->dri_output", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "Null mediaDrvCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mediaCtx->pGmmClientContext, "Null mediaCtx->pGmmClientContext", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaceId", VA_STATUS_ERROR_INVALID_SURFACE);

    struct dri_vtable * const dri_vtable = &mediaCtx->dri_output->vtable;
    DDI_CHK_NULL(dri_vtable, "Null dri_vtable", VA_STATUS_ERROR_INVALID_PARAMETER);

    dri_drawable = dri_vtable->get_drawable(ctx, (Drawable)draw);
    DDI_CHK_NULL(dri_drawable, "Null dri_drawable", VA_STATUS_ERROR_INVALID_PARAMETER);
    buffer = dri_vtable->get_rendering_buffer(ctx, dri_drawable);
    DDI_CHK_NULL(buffer, "Null buffer", VA_STATUS_ERROR_INVALID_PARAMETER);

    bufferObject = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(bufferObject, "Null bufferObject", VA_STATUS_ERROR_INVALID_SURFACE);
    DdiMediaUtil_MediaPrintFps();
    pitch = bufferObject->iPitch;

    vpCtx         = nullptr;
    if (nullptr != mediaCtx->pVpCtxHeap->pHeapBase)
    {
        vpCtx = (PDDI_VP_CONTEXT)DdiMedia_GetContextFromContextID(ctx, (VAContextID)(0 + DDI_MEDIA_VACONTEXTID_OFFSET_VP), &ctxType);
        DDI_CHK_NULL(vpCtx, "Null vpCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
        vpHal = vpCtx->pVpHal;
        DDI_CHK_NULL(vpHal, "Null vpHal", VA_STATUS_ERROR_INVALID_PARAMETER);
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    // Zero memory
    MOS_ZeroMemory(&Surf,    sizeof(Surf));
    MOS_ZeroMemory(&target, sizeof(target));
    MOS_ZeroMemory(&renderParams, sizeof(renderParams));
    MOS_ZeroMemory(&gmmParams, sizeof(gmmParams));

    renderParams.Component = COMPONENT_LibVA;

    //Init source rectangle
    Rect_init(&srcRect, srcx, srcy, srcw, srch);
    Rect_init(&dstRect, destx, desty, destw, desth);
    
    if( destx + destw > dri_drawable->x + dri_drawable->width )
    {
        //return VA_STATUS_ERROR_INVALID_PARAMETER;
        dstRect.right = dri_drawable->x + dri_drawable->width - destx;
        if(dstRect.right <= 0)
        {
            return VA_STATUS_SUCCESS;
        }
    }
    if(desty + desth > dri_drawable->y + dri_drawable->height) 
    {
        dstRect.bottom = dri_drawable->y + dri_drawable->height - desty;
        if(dstRect.bottom <= 0)
        {
            return VA_STATUS_SUCCESS;
        }
    }
    // Source Surface Information
    Surf.Format                 = VpGetFormatFromMediaFormat(bufferObject->format);           // Surface format
    Surf.SurfType               = SURF_IN_PRIMARY;       // Surface type (context)
    Surf.SampleType             = SAMPLE_PROGRESSIVE;
    Surf.ScalingMode            = VPHAL_SCALING_AVS;

    Surf.OsResource.Format      = VpGetFormatFromMediaFormat(bufferObject->format);
    Surf.OsResource.iWidth      = bufferObject->iWidth;
    Surf.OsResource.iHeight     = bufferObject->iHeight;
    Surf.OsResource.iPitch      = bufferObject->iPitch;
    Surf.OsResource.iCount      = 0;
    Surf.OsResource.TileType    = VpGetTileTypeFromMediaTileType(bufferObject->TileType);
    Surf.OsResource.bMapped     = bufferObject->bMapped;
    Surf.OsResource.bo          = bufferObject->bo;
    Surf.OsResource.pGmmResInfo = bufferObject->pGmmResourceInfo;

    Surf.dwWidth                = bufferObject->iWidth;
    Surf.dwHeight               = bufferObject->iHeight;
    Surf.dwPitch                = bufferObject->iPitch;
    Surf.TileType               = VpGetTileTypeFromMediaTileType(bufferObject->TileType);
    Surf.ColorSpace             = DdiVp_GetColorSpaceFromMediaFormat(bufferObject->format);
    Surf.ExtendedGamut          = false;
    Surf.rcSrc                  = srcRect;
    Surf.rcDst                  = dstRect;

    MOS_LINUX_BO* drawable_bo = mos_bo_create_from_name(mediaCtx->pDrmBufMgr, "rendering buffer", buffer->dri2.name);


    if  (nullptr == drawable_bo)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (!mos_bo_get_tiling(drawable_bo, &drawable_tiling_mode, &drawable_swizzle_mode))
    {
        switch (drawable_tiling_mode)
        {
        case TILING_Y:
           tileType = MOS_TILE_Y;
           break;
        case TILING_X:
           tileType = MOS_TILE_X;
           gmmParams.Flags.Info.TiledX    = true;
           break;
        case TILING_NONE:
           tileType = MOS_TILE_LINEAR;
           gmmParams.Flags.Info.Linear    = true;
           break;
        default:
           drawable_tiling_mode = TILING_NONE;
           tileType = MOS_TILE_LINEAR;
           gmmParams.Flags.Info.Linear    = true;
           break;
        }
    }
    else
    {
        target.OsResource.TileType = (MOS_TILE_TYPE)TILING_NONE;
        tileType = MOS_TILE_LINEAR;
        gmmParams.Flags.Info.Linear    = true;
    }
    gmmParams.Flags.Info.LocalOnly = MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrLocalMemory);

    target.Format                = Format_A8R8G8B8;
    target.SurfType              = SURF_OUT_RENDERTARGET;

    //init target retangle
    Rect_init(&srcRect, dri_drawable->x, dri_drawable->y, dri_drawable->width, dri_drawable->height);
    Rect_init(&dstRect, dri_drawable->x, dri_drawable->y, dri_drawable->width, dri_drawable->height);

    // Create GmmResourceInfo
    gmmParams.Flags.Gpu.Video       = true;
    gmmParams.BaseWidth             = dri_drawable->width;
    gmmParams.BaseHeight            = dri_drawable->height;
    gmmParams.ArraySize             = 1;
    gmmParams.Type                  = RESOURCE_2D;
    gmmParams.Format                = GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
    //gmmParams.Format                = GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
    target.OsResource.pGmmResInfo = mediaCtx->pGmmClientContext->CreateResInfoObject(&gmmParams);
    if (nullptr == target.OsResource.pGmmResInfo)
    {
        mos_bo_unreference(drawable_bo);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    target.OsResource.iWidth     = dri_drawable->width;
    target.OsResource.iHeight    = dri_drawable->height;
    target.OsResource.iPitch     = buffer->dri2.pitch;
    target.OsResource.Format     = Format_A8R8G8B8;
    target.OsResource.iCount     = 0;
    target.OsResource.bo         = drawable_bo;
    target.OsResource.pData      = (uint8_t *)drawable_bo->virt;
    target.OsResource.TileType   = tileType;
    target.TileType              = tileType;
    target.dwWidth               = dri_drawable->width;
    target.dwHeight              = dri_drawable->height;
    target.dwPitch               = target.OsResource.iPitch;
    target.ColorSpace            = CSpace_sRGB;
    target.ExtendedGamut         = false;
    target.rcSrc                 = srcRect;
    target.rcDst                 = dstRect;

    renderParams.uSrcCount                  = 1;
    renderParams.uDstCount                  = 1;
    renderParams.pSrc[0]                    = &Surf;
    renderParams.pTarget[0]                 = &target;
    renderParams.pColorFillParams           = &colorFill;
    renderParams.pColorFillParams->Color    = 0xFF000000;
    renderParams.pColorFillParams->bYCbCr   = false;
    renderParams.pColorFillParams->CSpace   = CSpace_sRGB;

    DdiMediaUtil_LockMutex(&mediaCtx->PutSurfaceRenderMutex);
    eStatus = vpHal->Render(&renderParams);
    if (MOS_FAILED(eStatus))
    {
        DdiMediaUtil_UnLockMutex(&mediaCtx->PutSurfaceRenderMutex);
        mos_bo_unreference(drawable_bo);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    DdiMediaUtil_UnLockMutex(&mediaCtx->PutSurfaceRenderMutex);
    mos_bo_unreference(drawable_bo);
    target.OsResource.bo         = nullptr;
    DdiMediaUtil_LockMutex(&mediaCtx->PutSurfaceSwapBufferMutex);
    dri_vtable->swap_buffer(ctx, dri_drawable);
    DdiMediaUtil_UnLockMutex(&mediaCtx->PutSurfaceSwapBufferMutex);

    mediaCtx->pGmmClientContext->DestroyResInfoObject(target.OsResource.pGmmResInfo);
    target.OsResource.pGmmResInfo = nullptr;

    return VA_STATUS_SUCCESS;
}

#ifndef ANDROID
// move from media_libva_putsurface_linux.c
static unsigned long DdiMedia_mask2shift(unsigned long mask)
{
    unsigned long shift = 0;
    while((mask & 0x1) == 0)
    {
        mask = mask >> 1;
        shift++;
    }
    return shift;
}
static void DdiMedia_yuv2pixel(uint32_t *pixel, int32_t y, int32_t u, int32_t v,
                               unsigned long rshift, unsigned long rmask,
                               unsigned long gshift, unsigned long gmask,
                               unsigned long bshift, unsigned long bmask)
{
    DDI_CHK_NULL(pixel, "nullptr pixel", );
    /* Warning, magic values ahead */
    int32_t r = y + ((351 * (v-128)) >> 8);
    int32_t g = y - (((179 * (v-128)) + (86 * (u-128))) >> 8);
    int32_t b = y + ((444 * (u-128)) >> 8);

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (r < 0)   r = 0;
    if (g < 0)   g = 0;
    if (b < 0)   b = 0;

    *pixel = (uint32_t)(((r << rshift) & rmask) | ((g << gshift) & gmask) |((b << bshift) & bmask));
}

#define YUV_444P_TO_ARGB() \
    srcY = umdContextY + pitch * srcy;\
    srcU = srcY + pitch * ((mediaSurface->iHeight));\
    srcV = srcU + pitch * ((mediaSurface->iHeight));\
     \
    for(y = srcy; y < (srcy + height); y += 1) \
    {\
        for(x = srcx; x < (srcx + width); x += 1) \
        {\
            y1 = *(srcY + x); \
            u1 = *(srcU + x);\
            v1 = *(srcV + x);\
            \
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + (x * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
           \
        }\
        srcY += pitch;\
        srcU += pitch;\
        srcV += pitch;\
    }

#define YUV_422H_TO_ARGB()\
    srcY = umdContextY + pitch * srcy;\
    srcU = srcY + pitch * mediaSurface->iHeight;\
    srcV = srcU + pitch * mediaSurface->iHeight;\
    \
    for(y = srcy; y < (srcy + height); y += 1)\
    {\
        for(x = srcx; x < (srcx + width); x += 2)\
        {\
            y1 = *(srcY + x);\
            y2 = *(srcY + x + 1);\
            u1 = *(srcU + x / 2);\
            v1 = *(srcV + x / 2);\
            \
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + (x * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + ((x + 1) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
        }\
        srcY += pitch;\
        srcU += pitch;\
        srcV += pitch;\
    }

#define YUV_422V_TO_ARGB() \
    srcY = umdContextY + pitch * srcy;\
    srcU = srcY + pitch * mediaSurface->iHeight;\
    srcV = srcU + pitch * mediaSurface->iHeight / 2;\
    \
    for(y = srcy; y < (srcy + width); y += 1)\
    {\
        for(x = srcx; x < (srcx + height); x += 2)\
        {\
            y1 = *(srcY + x * pitch);\
            y2 = *(srcY + (x + 1) * pitch);\
            u1 = *(srcU + (x / 2) * pitch);\
            v1 = *(srcV + (x / 2) * pitch);\
            \
            pixel = (uint32_t *)(ximg->data + (x * ximg->bytes_per_line) + (y * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + (x* ximg->bytes_per_line) + ((y + 1) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            \
        }\
        \
        srcY += 1;\
        srcU += 1;\
        srcV += 1;\
    }

#define YUV_IMC3_TO_ARGB() \
    srcY = umdContextY + pitch * srcy;\
    srcU = srcY + pitch * mediaSurface->iHeight;\
    srcV = srcU + pitch * mediaSurface->iHeight / 2;\
    \
    for(y = srcy; y < (srcy + height); y += 2) \
    {\
        for(x = srcx; x < (srcx + width); x += 2) \
        {\
            y1 = *(srcY + x);\
            y2 = *(srcY + x + 1);\
            y3 = *(srcY + x + pitch);\
            y4 = *(srcY + x + pitch + 1);\
            \
            u1 = *(srcU + x / 2);\
            v1 = *(srcV + x / 2);\
            \
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + (x * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + ((x + 1) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + ((y + 1) * ximg->bytes_per_line) + (x * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y3, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + ((y + 1) * ximg->bytes_per_line) + ((x + 1) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y4, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask); \
        }\
        srcY += pitch * 2;\
        srcU += pitch;\
        srcV += pitch;\
    }

#define YUV_411P_TO_ARGB() \
    srcY = umdContextY + pitch * srcy;\
    srcU = srcY + pitch * mediaSurface->iHeight;\
    srcV = srcU + pitch * mediaSurface->iHeight;\
    \
    for(y = srcy; y < (srcy + height); y += 1)\
    {\
        for(x = srcx; x < (srcx + width); x += 4)\
        {\
            y1 = *(srcY + x);\
            y2 = *(srcY + x + 1);\
            y3 = *(srcY + x + 2);\
            y4 = *(srcY + x + 3);\
            \
            u1 = *(srcU + x / 4);\
            v1 = *(srcV + x / 4);\
            \
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + (x * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + ((x + 1) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + ((y ) * ximg->bytes_per_line) + ((x+2) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y3, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + ((y) * ximg->bytes_per_line) + ((x + 3) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y4, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
        }\
        srcY  += pitch;\
        srcU  += pitch;\
        srcV  += pitch;\
    }

#define YUV_400P_TO_ARGB()\
    srcY = umdContextY + pitch * srcy;\
    srcU = srcY;\
    srcV = srcY;\
    \
    for(y = srcy; y < (srcy + height); y += 2)\
    {\
        for(x = srcx; x < (srcx + width); x += 2)\
        {\
            y1 = *(srcY + x);\
            y2 = *(srcY + x + 1);\
            y3 = *(srcY + x + pitch);\
            y4 = *(srcY + x + pitch + 1);\
            \
            u1 = 128;\
            v1 = 128;\
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + (x * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + ((x + 1) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + ((y + 1) * ximg->bytes_per_line) + (x * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y3, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + ((y + 1) * ximg->bytes_per_line) + ((x + 1) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y4, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
        }\
        srcY += pitch * 2;\
        srcU += pitch;\
        srcV += pitch;\
    }

#define YUV_NV12_TO_ARGB()\
    srcY = umdContextY + pitch * srcy;\
    srcU = srcY + pitch * mediaSurface->iHeight;\
    srcV = srcU + 1;\
    \
    for(y = srcy; y < (srcy + height); y += 2)\
    {\
        for(x = srcx; x < (srcx + width); x += 2)\
        {\
            y1 = *(srcY + x);\
            y2 = *(srcY + x + 1);\
            y3 = *(srcY + x + pitch);\
            y4 = *(srcY + x + pitch + 1);\
            \
            u1 = *(srcU + x);\
            v1 = *(srcU + x +1);\
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + (x * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + (y * ximg->bytes_per_line) + ((x + 1) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + ((y + 1) * ximg->bytes_per_line) + (x * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y3, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pixel = (uint32_t *)(ximg->data + ((y + 1) * ximg->bytes_per_line) + ((x + 1) * (ximg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pixel, y4, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
        }\
        srcY += pitch * 2;\
        srcU += pitch;\
    }

VAStatus DdiMedia_PutSurfaceLinuxSW(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void*            draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
)
{
    PDDI_MEDIA_CONTEXT mediaCtx         = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->X11FuncTable, "nullptr X11FuncTable", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->X11FuncTable->pfnXCreateGC, "nullptr pfnXCreateGC", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->X11FuncTable->pfnXFreeGC, "nullptr pfnXFreeGC", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->X11FuncTable->pfnXCreateImage, "nullptr pfnXCreateImage", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->X11FuncTable->pfnXDestroyImage, "nullptr pfnXDestroyImage", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->X11FuncTable->pfnXPutImage, "nullptr pfnXPutImage", VA_STATUS_ERROR_INVALID_CONTEXT);

    TypeXCreateGC     pfn_XCreateGC     = (TypeXCreateGC)(mediaCtx->X11FuncTable->pfnXCreateGC);
    TypeXFreeGC       pfn_XFreeGC       = (TypeXFreeGC)(mediaCtx->X11FuncTable->pfnXFreeGC);
    TypeXCreateImage  pfn_XCreateImage  = (TypeXCreateImage)(mediaCtx->X11FuncTable->pfnXCreateImage);
    TypeXDestroyImage pfn_XDestroyImage = (TypeXDestroyImage)(mediaCtx->X11FuncTable->pfnXDestroyImage);
    TypeXPutImage     pfn_XPutImage     = (TypeXPutImage)(mediaCtx->X11FuncTable->pfnXPutImage);

    DDI_MEDIA_SURFACE *mediaSurface     = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface.", VA_STATUS_ERROR_INVALID_SURFACE);

    uint16_t width  = 0;
    if (srcw <= destw)
        width = srcw;
    else
        width = destw;

    uint16_t height = 0;
    if (srch <= desth)
        height = srch;
    else
        height = desth;

    int32_t  pitch   = mediaSurface->iPitch;
    uint32_t adjustU = 1;
    uint32_t adjustD = 1;
    switch(mediaSurface->format)
    {
        case Media_Format_422H:
        case Media_Format_444P:
        case Media_Format_411P:
            adjustU = 3;
            adjustD = 1;
            break;
        case Media_Format_400P:
            adjustU = 1;
            adjustD = 1;
            break;
        case Media_Format_422V:
        case Media_Format_IMC3:
            adjustU = 2;
            adjustD = 1;
            break;
        case Media_Format_NV12:
            adjustU = 3;
            adjustD = 2;
            break;
        default:
            DDI_ASSERTMESSAGE("Color Format is not supported: %d",mediaSurface->format);
            return VA_STATUS_ERROR_INVALID_VALUE;
    }

    uint32_t surfaceSize          = pitch * mediaSurface->iHeight * adjustU / adjustD;
    uint8_t  *dispTempBuffer      = (uint8_t *)malloc(surfaceSize);
    if (dispTempBuffer == nullptr)
    {
        DdiMediaUtil_UnlockSurface(mediaSurface);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    uint8_t *umdContextY = dispTempBuffer;
    uint8_t *ptr         = (uint8_t*)DdiMediaUtil_LockSurface(mediaSurface, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));
    MOS_STATUS eStatus   = MOS_SecureMemcpy(umdContextY, surfaceSize, ptr, surfaceSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_FreeMemory(dispTempBuffer);
        DDI_ASSERTMESSAGE("DDI:Failed to copy surface buffer data!");
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    Visual *visual       = DefaultVisual(ctx->native_dpy, ctx->x11_screen);
    GC     gc            = (*pfn_XCreateGC)((Display*)ctx->native_dpy, (Drawable)draw, 0, nullptr);

    if (TrueColor != visual->c_class)
    {
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        MOS_FreeMemory(dispTempBuffer);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    unsigned long rmask  = visual->red_mask;
    unsigned long gmask  = visual->green_mask;
    unsigned long bmask  = visual->blue_mask;

    unsigned long rshift = DdiMedia_mask2shift(rmask);
    unsigned long gshift = DdiMedia_mask2shift(gmask);
    unsigned long bshift = DdiMedia_mask2shift(bmask);

    int32_t depth        = DefaultDepth(ctx->native_dpy, ctx->x11_screen);
    XImage   *ximg  = (*pfn_XCreateImage)((Display*)ctx->native_dpy, visual, depth, ZPixmap, 0, nullptr,width, height, 32, 0 );
    if (ximg == nullptr)
    {
        MOS_FreeMemory(dispTempBuffer);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (ximg->bits_per_pixel != 32)
    {
         (*pfn_XDestroyImage)(ximg);
         (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
         MOS_FreeMemory(dispTempBuffer);
         return VA_STATUS_ERROR_UNKNOWN;
    }

    ximg->data = (char *) malloc(ximg->bytes_per_line * MOS_ALIGN_CEIL(height, 2)); // If height is odd, need to add it by one for we process two lines per iteration
    if (nullptr == ximg->data)
    {
        (*pfn_XDestroyImage)(ximg);
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        MOS_FreeMemory(dispTempBuffer);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    int32_t  x = 0;
    int32_t  y = 0;
    uint8_t  *srcY = nullptr;
    uint8_t  *srcU = nullptr;
    uint8_t  *srcV = nullptr;
    uint32_t *pixel = nullptr;
    int32_t  y1 = 0, y2 = 0, y3 = 0, y4 = 0, u1 = 0, v1 = 0;
     switch(mediaSurface->format)
    {
        case Media_Format_444P:
            YUV_444P_TO_ARGB();
            break;
        case Media_Format_422H:
            YUV_422H_TO_ARGB();
            break;
        case Media_Format_422V:
            YUV_422V_TO_ARGB();
            break;
        case Media_Format_IMC3:
            YUV_IMC3_TO_ARGB();
            break;
        case Media_Format_411P:
            YUV_411P_TO_ARGB();
            break;
        case Media_Format_400P:
            YUV_400P_TO_ARGB();
            break;
        case Media_Format_NV12:
            YUV_NV12_TO_ARGB();
            break;
        default:
            DDI_ASSERTMESSAGE("Color Format is not supported: %d", mediaSurface->format);
    }

    DdiMediaUtil_UnlockSurface(mediaSurface);

    (*pfn_XPutImage)((Display*)ctx->native_dpy,(Drawable)draw, gc, ximg, 0, 0, destx, desty, destw, desth);

    if (ximg != nullptr)
    {
        (*pfn_XDestroyImage)(ximg);
    }
    (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
    MOS_FreeMemory(dispTempBuffer);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_PutSurfaceDummy(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void            *draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

#endif
