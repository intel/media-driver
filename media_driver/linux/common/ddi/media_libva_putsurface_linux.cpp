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
    struct dso_handle *h;

    h = (dso_handle*) calloc(1, sizeof(*h));
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
    dso_generic_func func; 
    dso_generic_func * const func_ptr = (dso_generic_func*) func_vptr;
    const char *error;

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

/* Loads function name from vtable */
bool
dso_get_symbols(
    struct dso_handle          *h,
    void                       *vtable,
    uint32_t                    vtable_length,
    const struct dso_symbol    *symbols
)
{
    const struct dso_symbol *s;
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
    PDDI_MEDIA_CONTEXT pMediaDrvCtx;
    pMediaDrvCtx = DdiMedia_GetMediaContext(ctx);

    struct dso_handle *dso_handle;
    struct dri_vtable *dri_vtable;

    pMediaDrvCtx->dri_output = nullptr;

    static const struct dso_symbol symbols[] = {
        { "dri_get_drawable",
          offsetof(struct dri_vtable, get_drawable) },
        { "dri_get_rendering_buffer",
          offsetof(struct dri_vtable, get_rendering_buffer) },
        { "dri_swap_buffer",
          offsetof(struct dri_vtable, swap_buffer) },
        { nullptr, }
    };

    pMediaDrvCtx->dri_output = (va_dri_output*) calloc(1, sizeof(struct va_dri_output));
    if (!pMediaDrvCtx->dri_output){
        goto error;
    }

    pMediaDrvCtx->dri_output->handle = dso_open(LIBVA_X11_NAME);
    if (!pMediaDrvCtx->dri_output->handle){
        free(pMediaDrvCtx->dri_output);
        pMediaDrvCtx->dri_output = nullptr;
        goto error;
    }

    dso_handle = pMediaDrvCtx->dri_output->handle;
    dri_vtable = &pMediaDrvCtx->dri_output->vtable;
    if (!dso_get_symbols(dso_handle, dri_vtable, sizeof(*dri_vtable), symbols)){
        dso_close(pMediaDrvCtx->dri_output->handle);
        free(pMediaDrvCtx->dri_output);
        pMediaDrvCtx->dri_output = nullptr;
        goto error;
    }
    return true;

error:
    return false;
}

void
inline Rect_init(
    RECT            *Rect,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth
)
{
	if (nullptr == Rect) 
    {
        return;
    }
    Rect->left                    = destx;
    Rect->top                     = desty;
    Rect->right                   = destw;
    Rect->bottom                  = desth;
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
    XImage*                   pXimg;
    int32_t                   surf_width;
    int32_t                   surf_height;
    PDDI_MEDIA_CONTEXT        pMediaDrvCtx;
    PDDI_MEDIA_SURFACE        pDstSurfBuffObj;

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
    pXimg                     = nullptr;
    pMediaDrvCtx              = DdiMedia_GetMediaContext(ctx);
    pDstSurfBuffObj           = DdiMedia_GetSurfaceFromVASurfaceID(pMediaDrvCtx, surface);

    if (nullptr == pDstSurfBuffObj)
    {
        return VA_STATUS_ERROR_UNKNOWN;
    }

    if (nullptr == pMediaDrvCtx->X11FuncTable                   ||
        nullptr == pMediaDrvCtx->X11FuncTable->pfnXCreateGC     ||
        nullptr == pMediaDrvCtx->X11FuncTable->pfnXFreeGC       ||
        nullptr == pMediaDrvCtx->X11FuncTable->pfnXCreateImage  ||
        nullptr == pMediaDrvCtx->X11FuncTable->pfnXDestroyImage ||
        nullptr == pMediaDrvCtx->X11FuncTable->pfnXPutImage)
    {
        return VA_STATUS_ERROR_UNKNOWN;
    }

    pfn_XCreateGC     = (TypeXCreateGC)(pMediaDrvCtx->X11FuncTable->pfnXCreateGC);
    pfn_XFreeGC       = (TypeXFreeGC)(pMediaDrvCtx->X11FuncTable->pfnXFreeGC);
    pfn_XCreateImage  = (TypeXCreateImage)(pMediaDrvCtx->X11FuncTable->pfnXCreateImage);
    pfn_XDestroyImage = (TypeXDestroyImage)(pMediaDrvCtx->X11FuncTable->pfnXDestroyImage);
    pfn_XPutImage     = (TypeXPutImage)(pMediaDrvCtx->X11FuncTable->pfnXPutImage);

    surf_width  = pDstSurfBuffObj->iWidth;
    surf_height = pDstSurfBuffObj->iHeight;

    visual = DefaultVisual(ctx->native_dpy, ctx->x11_screen);
    gc     = (*pfn_XCreateGC)((Display*)ctx->native_dpy, (Drawable)draw, 0, nullptr);
    depth  = DefaultDepth(ctx->native_dpy, ctx->x11_screen);

    if (TrueColor != visual->c_class) 
    {
        DDI_ASSERTMESSAGE("Default visual of X display must be TrueColor.");
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    pXimg = (*pfn_XCreateImage)((Display*)ctx->native_dpy, visual, depth, ZPixmap, 0, nullptr,surf_width, surf_height, 32, 0 );

    if (nullptr == pXimg) 
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (pXimg->bits_per_pixel != 32) 
    {
        DDI_ASSERTMESSAGE("Display uses %d bits/pixel this not supported.",pXimg->bits_per_pixel);
        (*pfn_XDestroyImage)(pXimg);
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    pXimg->data = (char *)DdiMediaUtil_LockSurface(pDstSurfBuffObj, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));

    if (nullptr == pXimg->data) 
    {
        DdiMediaUtil_UnlockSurface(pDstSurfBuffObj);
        (*pfn_XDestroyImage)(pXimg);
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    (*pfn_XPutImage)((Display*)ctx->native_dpy, (Drawable)draw, gc, pXimg, 0, 0, destx, desty, surf_width, surf_height);

    DdiMediaUtil_UnlockSurface(pDstSurfBuffObj);
    pXimg->data = nullptr;

    if (nullptr != pXimg)
    {
        (*pfn_XDestroyImage)(pXimg);
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
    VphalState             *pVpHal = nullptr;
    int32_t                 OvRenderIndex = 0;
    static VPHAL_SURFACE    Surf;
    VPHAL_SURFACE           Target;
    VPHAL_RENDER_PARAMS     RenderParams;

    MOS_STATUS              eStatus = MOS_STATUS_INVALID_PARAMETER;
    RECT                    Rect = { 0, 0, 180, 120 };
    RECT                    DstRect = { 0, 0, 180, 120 };
    PDDI_MEDIA_CONTEXT      pMediaCtx;
    PDDI_MEDIA_SURFACE      pBufferObject;
    uint32_t                width,height,pitch;
    uint32_t                drawable_tiling_mode;
    uint32_t                drawable_swizzle_mode;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;
    MOS_TILE_TYPE           tileType;

    uint32_t                uiCtxType;
    PDDI_VP_CONTEXT         pVpCtx;
    struct dri_drawable*    dri_drawable;
    union dri_buffer*       buffer;


    GMM_RESCREATE_PARAMS    GmmParams;

    pMediaCtx     = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->dri_output, "Null pMediaDrvCtx->dri_output", VA_STATUS_ERROR_INVALID_PARAMETER);
	DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaDrvCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LESS((uint32_t)surface, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaceId", VA_STATUS_ERROR_INVALID_SURFACE);

	struct dri_vtable * const dri_vtable = &pMediaCtx->dri_output->vtable;
    dri_drawable = dri_vtable->get_drawable(ctx, (Drawable)draw);
    assert(dri_drawable);
    buffer = dri_vtable->get_rendering_buffer(ctx, dri_drawable);
    assert(buffer);

    pBufferObject = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surface);
    DDI_CHK_NULL(pBufferObject, "Null pBufferObject", VA_STATUS_ERROR_INVALID_SURFACE);
    DdiMediaUtil_MediaPrintFps();
    pitch = pBufferObject->iPitch;
   
    pVpCtx         = nullptr;
    if (nullptr != pMediaCtx->pVpCtxHeap->pHeapBase)
    {
        pVpCtx = (PDDI_VP_CONTEXT)DdiMedia_GetContextFromContextID(ctx, (VAContextID)(0 + DDI_MEDIA_VACONTEXTID_OFFSET_VP), &uiCtxType);
        DDI_CHK_NULL(pVpCtx, "Null pVpCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
        pVpHal = pVpCtx->pVpHal;
        DDI_CHK_NULL(pVpHal, "Null pVpHal", VA_STATUS_ERROR_INVALID_PARAMETER);
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    // Zero memory
    MOS_ZeroMemory(&Surf,    sizeof(Surf));
    MOS_ZeroMemory(&Target, sizeof(Target));
    MOS_ZeroMemory(&RenderParams, sizeof(RenderParams));
    MOS_ZeroMemory(&GmmParams, sizeof(GmmParams));

    RenderParams.Component = COMPONENT_LibVA;

   //Init source rectagle
    Rect_init(&Rect, 0, 0, srcw, srch);
    Rect_init(&DstRect, dri_drawable->x, dri_drawable->y, dri_drawable->width, dri_drawable->height);
    
    // Source Surface Information
    Surf.Format               = VpGetFormatFromMediaFormat(pBufferObject->format);           // Surface format
    Surf.SurfType             = SURF_IN_PRIMARY;       // Surface type (context)
    Surf.SampleType           = SAMPLE_PROGRESSIVE;
    Surf.ScalingMode          = VPHAL_SCALING_AVS;

    Surf.OsResource.Format      = VpGetFormatFromMediaFormat(pBufferObject->format);
    Surf.OsResource.iWidth      = pBufferObject->iWidth;
    Surf.OsResource.iHeight     = pBufferObject->iHeight;
    Surf.OsResource.iPitch      = pBufferObject->iPitch;
    Surf.OsResource.iCount      = 0;
    Surf.OsResource.TileType    = VpGetTileTypeFromMediaTileType(pBufferObject->TileType);
    Surf.OsResource.bMapped     = pBufferObject->bMapped;
    Surf.OsResource.bo          = pBufferObject->bo;
    Surf.OsResource.pGmmResInfo = pBufferObject->pGmmResourceInfo;

    Surf.ColorSpace            = DdiVp_GetColorSpaceFromMediaFormat(pBufferObject->format);
    Surf.ExtendedGamut         = false;
    Surf.rcSrc                 = Rect;
    Surf.rcDst                 = DstRect;

    MOS_LINUX_BO* drawable_bo = mos_bo_gem_create_from_name(pMediaCtx->pDrmBufMgr, "rendering buffer", buffer->dri2.name);
    if  (nullptr == drawable_bo)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (!mos_bo_get_tiling(drawable_bo, &drawable_tiling_mode, &drawable_swizzle_mode))
    {
        switch (drawable_tiling_mode)
        {
        case I915_TILING_Y:
           tileType = MOS_TILE_Y;
           GmmParams.Flags.Info.TiledY    = true; 
		   break;
        case I915_TILING_X:
           tileType = MOS_TILE_X;
           GmmParams.Flags.Info.TiledX    = true;
		   break;
        case I915_TILING_NONE:
           tileType = MOS_TILE_LINEAR;
           GmmParams.Flags.Info.Linear    = true;
           break;
        default:
           drawable_tiling_mode = I915_TILING_NONE;
           tileType = MOS_TILE_LINEAR;
           GmmParams.Flags.Info.Linear    = true;
           break;
        Target.OsResource.TileType = (MOS_TILE_TYPE)drawable_tiling_mode;
        }
    }
    else
    {
        Target.OsResource.TileType = (MOS_TILE_TYPE)I915_TILING_NONE;
        tileType = MOS_TILE_LINEAR;
        GmmParams.Flags.Info.Linear    = true;
    }

    Target.Format                = Format_A8R8G8B8;
    Target.SurfType              = SURF_OUT_RENDERTARGET;

    //init target retangle
    Rect_init(&Rect, 0, 0, dri_drawable->width, dri_drawable->height);
    Rect_init(&DstRect, dri_drawable->x, dri_drawable->y, dri_drawable->width, dri_drawable->height);

    // Create GmmResourceInfo
    GmmParams.Flags.Gpu.Video       = true;
    GmmParams.BaseWidth             = dri_drawable->width;
    GmmParams.BaseHeight            = dri_drawable->height;
    GmmParams.ArraySize             = 1;
    GmmParams.Type                  = RESOURCE_2D;
    GmmParams.Format                = GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
    //GmmParams.Format                = GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
    Target.OsResource.pGmmResInfo   = GmmResCreate(&GmmParams);
    if (nullptr == Target.OsResource.pGmmResInfo)
    {
        mos_bo_unreference(drawable_bo);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    Target.OsResource.iWidth     = dri_drawable->width;
    Target.OsResource.iHeight    = dri_drawable->height;
    Target.OsResource.iPitch     = buffer->dri2.pitch;
    Target.OsResource.Format     = Format_A8R8G8B8;
    Target.OsResource.iCount     = 0;
    Target.OsResource.bo         = drawable_bo;
    Target.OsResource.pData      = (uint8_t *)drawable_bo->virt;
    Target.OsResource.TileType   = tileType;
    Target.dwWidth               = dri_drawable->width;
    Target.dwHeight              = dri_drawable->height;
    Target.dwPitch               = Target.OsResource.iPitch;
    Target.ColorSpace            = CSpace_sRGB;
    Target.ExtendedGamut         = false;
    Target.rcSrc                 = Rect;
    Target.rcDst                 = DstRect;

    RenderParams.uSrcCount          = 1;
    RenderParams.uDstCount          = 1;
    RenderParams.pSrc[0]            = &Surf;
    RenderParams.pTarget[0]         = &Target;

    DdiMediaUtil_LockMutex(&pMediaCtx->PutSurfaceRenderMutex);
    eStatus = pVpHal->Render(&RenderParams);
    if (MOS_FAILED(eStatus))
    {
        DdiMediaUtil_UnLockMutex(&pMediaCtx->PutSurfaceRenderMutex);
        mos_bo_unreference(drawable_bo);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    DdiMediaUtil_UnLockMutex(&pMediaCtx->PutSurfaceRenderMutex);
    mos_bo_unreference(drawable_bo);
    Target.OsResource.bo         = nullptr;
    DdiMediaUtil_LockMutex(&pMediaCtx->PutSurfaceSwapBufferMutex);
    dri_vtable->swap_buffer(ctx, dri_drawable);
    DdiMediaUtil_UnLockMutex(&pMediaCtx->PutSurfaceSwapBufferMutex);

    GmmResFree(Target.OsResource.pGmmResInfo);
    Target.OsResource.pGmmResInfo = nullptr;
    
    return VA_STATUS_SUCCESS;
}

