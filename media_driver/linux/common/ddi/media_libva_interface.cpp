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
//! \file     media_libva_interface.cpp
//! \brief    libva interface implementaion.
//!

#include "media_libva_interface.h"
#include "media_libva.h"
#include "media_ddi_prot.h"
#include "media_libva_encoder.h"
#include "media_libva_util.h"
#include "media_capstable_specific.h"
#include "media_libva_interface_next.h"
#include "media_capstable_specific.h"
#include "media_libva_caps_next.h"

VAStatus MediaLibvaInterface::LoadFunction(VADriverContextP ctx)
{
    DDI_CHK_NULL(ctx,         "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);

    struct VADriverVTable    *pVTable     = DDI_CODEC_GET_VTABLE(ctx);
    DDI_CHK_NULL(pVTable,     "nullptr pVTable",      VA_STATUS_ERROR_INVALID_CONTEXT);

    struct VADriverVTableVPP *pVTableVpp  = DDI_CODEC_GET_VTABLE_VPP(ctx);
    DDI_CHK_NULL(pVTableVpp,  "nullptr pVTableVpp",   VA_STATUS_ERROR_INVALID_CONTEXT);

#if VA_CHECK_VERSION(1,11,0)
    struct VADriverVTableProt *pVTableProt = DDI_CODEC_GET_VTABLE_PROT(ctx);
    DDI_CHK_NULL(pVTableProt,  "nullptr pVTableProt",   VA_STATUS_ERROR_INVALID_CONTEXT);
#endif

    ctx->version_major                       = VA_MAJOR_VERSION;
    ctx->version_minor                       = VA_MINOR_VERSION;
    ctx->max_profiles                        = DDI_CODEC_GEN_MAX_PROFILES;
    ctx->max_entrypoints                     = DDI_CODEC_GEN_MAX_ENTRYPOINTS;
    ctx->max_attributes                      = (int32_t)VAConfigAttribTypeMax;
    ctx->max_subpic_formats                  = DDI_CODEC_GEN_MAX_SUBPIC_FORMATS;
    ctx->max_display_attributes              = DDI_MEDIA_GEN_MAX_DISPLAY_ATTRIBUTES;
    ctx->str_vendor                          = DDI_CODEC_GEN_STR_VENDOR;
    ctx->vtable_tpi                          = nullptr;

    pVTable->vaTerminate                     = Terminate;
    pVTable->vaQueryConfigEntrypoints        = QueryConfigEntrypoints;
    pVTable->vaQueryConfigProfiles           = QueryConfigProfiles;
    pVTable->vaQueryConfigAttributes         = QueryConfigAttributes;
    pVTable->vaCreateConfig                  = CreateConfig;
    pVTable->vaDestroyConfig                 = DestroyConfig;
    pVTable->vaGetConfigAttributes           = GetConfigAttributes;

    pVTable->vaCreateSurfaces                = MediaLibvaInterfaceNext::CreateSurfaces;
    pVTable->vaDestroySurfaces               = MediaLibvaInterfaceNext::DestroySurfaces;
    pVTable->vaCreateSurfaces2               = MediaLibvaInterfaceNext::CreateSurfaces2;

    pVTable->vaCreateContext                 = CreateContext;
    pVTable->vaDestroyContext                = DestroyContext;
    pVTable->vaCreateBuffer                  = CreateBuffer;
    pVTable->vaBufferSetNumElements          = MediaLibvaInterfaceNext::BufferSetNumElements;
    pVTable->vaMapBuffer                     = MapBuffer;
    pVTable->vaUnmapBuffer                   = UnmapBuffer;
    pVTable->vaDestroyBuffer                 = DestroyBuffer;
    pVTable->vaBeginPicture                  = BeginPicture;
    pVTable->vaRenderPicture                 = RenderPicture;
    pVTable->vaEndPicture                    = EndPicture;
    pVTable->vaSyncSurface                   = SyncSurface;
#if VA_CHECK_VERSION(1, 9, 0)
    pVTable->vaSyncSurface2                  = SyncSurface2;
    pVTable->vaSyncBuffer                    = MediaLibvaInterfaceNext::SyncBuffer;
#endif
    pVTable->vaQuerySurfaceStatus            = MediaLibvaInterfaceNext::QuerySurfaceStatus;
    pVTable->vaQuerySurfaceError             = QuerySurfaceError;
    pVTable->vaQuerySurfaceAttributes        = QuerySurfaceAttributes;
    pVTable->vaPutSurface                    = PutSurface;
    pVTable->vaQueryImageFormats             = QueryImageFormats;

    pVTable->vaCreateImage                   = MediaLibvaInterfaceNext::CreateImage;
    pVTable->vaDeriveImage                   = MediaLibvaInterfaceNext::DeriveImage;
    pVTable->vaDestroyImage                  = MediaLibvaInterfaceNext::DestroyImage;
    pVTable->vaSetImagePalette               = MediaLibvaInterfaceNext::SetImagePalette;
    pVTable->vaGetImage                      = GetImage;
    pVTable->vaPutImage                      = PutImage;
    pVTable->vaQuerySubpictureFormats        = MediaLibvaInterfaceNext::QuerySubpictureFormats;
    pVTable->vaCreateSubpicture              = MediaLibvaInterfaceNext::CreateSubpicture;
    pVTable->vaDestroySubpicture             = MediaLibvaInterfaceNext::DestroySubpicture;
    pVTable->vaSetSubpictureImage            = MediaLibvaInterfaceNext::SetSubpictureImage;
    pVTable->vaSetSubpictureChromakey        = MediaLibvaInterfaceNext::SetSubpictureChromakey;
    pVTable->vaSetSubpictureGlobalAlpha      = MediaLibvaInterfaceNext::SetSubpictureGlobalAlpha;
    pVTable->vaAssociateSubpicture           = MediaLibvaInterfaceNext::AssociateSubpicture;
    pVTable->vaDeassociateSubpicture         = MediaLibvaInterfaceNext::DeassociateSubpicture;
    pVTable->vaQueryDisplayAttributes        = MediaLibvaInterfaceNext::QueryDisplayAttributes;
    pVTable->vaGetDisplayAttributes          = MediaLibvaInterfaceNext::GetDisplayAttributes;
    pVTable->vaSetDisplayAttributes          = MediaLibvaInterfaceNext::SetDisplayAttributes;
    pVTable->vaQueryProcessingRate           = QueryProcessingRate;
#if VA_CHECK_VERSION(1,10,0)
    pVTable->vaCopy                          = MediaLibvaInterfaceNext::Copy;
#endif

    // vaTrace
    pVTable->vaBufferInfo                    = MediaLibvaInterfaceNext::BufferInfo;
    pVTable->vaLockSurface                   = LockSurface;
    pVTable->vaUnlockSurface                 = UnlockSurface;

    pVTableVpp->vaQueryVideoProcFilters      = QueryVideoProcFilters;
    pVTableVpp->vaQueryVideoProcFilterCaps   = QueryVideoProcFilterCaps;
    pVTableVpp->vaQueryVideoProcPipelineCaps = QueryVideoProcPipelineCaps;

#if VA_CHECK_VERSION(1,11,0)
    pVTableProt->vaCreateProtectedSession    = DdiMediaProtected::DdiMedia_CreateProtectedSession;
    pVTableProt->vaDestroyProtectedSession   = DdiMediaProtected::DdiMedia_DestroyProtectedSession;
    pVTableProt->vaAttachProtectedSession    = DdiMediaProtected::DdiMedia_AttachProtectedSession;
    pVTableProt->vaDetachProtectedSession    = DdiMediaProtected::DdiMedia_DetachProtectedSession;
    pVTableProt->vaProtectedSessionExecute   = DdiMediaProtected::DdiMedia_ProtectedSessionExecute;
#endif

    //pVTable->vaSetSurfaceAttributes          = DdiMedia_SetSurfaceAttributes;
    pVTable->vaGetSurfaceAttributes          = MediaLibvaInterfaceNext::GetSurfaceAttributes;
    //Export PRIMEFD/FLINK to application for buffer sharing with OpenCL/GL
    pVTable->vaAcquireBufferHandle           = MediaLibvaInterfaceNext::AcquireBufferHandle;
    pVTable->vaReleaseBufferHandle           = MediaLibvaInterfaceNext::ReleaseBufferHandle;
    pVTable->vaExportSurfaceHandle           = MediaLibvaInterfaceNext::ExportSurfaceHandle;
#ifndef ANDROID
    pVTable->vaCreateMFContext               = CreateMfeContextInternal;
    pVTable->vaMFAddContext                  = AddContextInternal;
    pVTable->vaMFReleaseContext              = ReleaseContextInternal;
    pVTable->vaMFSubmit                      = DdiEncode_MfeSubmit;
#endif

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterface::Terminate(VADriverContextP ctx)
{
    return DdiMedia_Terminate(ctx);
}

VAStatus MediaLibvaInterface::QueryConfigEntrypoints(
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      *entrypoint_list,
    int32_t           *num_entrypoints)
{
    DDI_FUNCTION_ENTER();

    // Because no implementation of APO DDI for decoding,
    // call legacy function to avoid cases failure
    // Once decoding is ready, call APO DDI.
    return DdiMedia_QueryConfigEntrypoints(ctx, profile, entrypoint_list,
            num_entrypoints);
}

VAStatus MediaLibvaInterface::QueryConfigProfiles(
    VADriverContextP  ctx,
    VAProfile         *profile_list,
    int32_t           *num_profiles)
{
    DDI_FUNCTION_ENTER();
    // Because no implementation of APO DDI for decoding,
    // call legacy function to avoid cases failure
    // Once decoding is ready, call APO DDI.
    return DdiMedia_QueryConfigProfiles(ctx, profile_list, num_profiles);
}

VAStatus MediaLibvaInterface::QueryConfigAttributes(
    VADriverContextP  ctx,
    VAConfigID        config_id,
    VAProfile         *profile,
    VAEntrypoint      *entrypoint,
    VAConfigAttrib    *attrib_list,
    int32_t           *num_attribs)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,   "nullptr ctx",     VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    auto IsSupportedByAPO = [&](){
        if (!mediaCtx->m_apoDdiEnabled)
        return false;

        //Temporarily disable APO for CP, remove when switch to CP DDI APO
        if(*profile == VAProfileProtected)
        {
           return false; 
        }

        if (mediaCtx->m_capsNext && mediaCtx->m_capsNext->m_capsTable)
        {
            ConfigLinux*  configItem = nullptr;
            configItem = mediaCtx->m_capsNext->m_capsTable->QueryConfigItemFromIndex(config_id);
            DDI_CHK_NULL(configItem, "Invalid config id!", false);
            return true;
        }
        else
        {
            return false;
        }
    };

    if(IsSupportedByAPO())
    {
        return MediaLibvaInterfaceNext::QueryConfigAttributes(ctx, config_id, profile, entrypoint, attrib_list, num_attribs);
    }
    else
    {
        return DdiMedia_QueryConfigAttributes(ctx, config_id, profile, entrypoint,
        attrib_list, num_attribs);
    }
}

VAStatus MediaLibvaInterface::CreateConfig(
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attrib_list,
    int32_t           num_attribs,
    VAConfigID        *config_id)
{
    DDI_FUNCTION_ENTER();
    
    DDI_CHK_NULL(ctx,   "nullptr ctx",     VA_STATUS_ERROR_INVALID_CONTEXT);

    // query profile and entrypoint from caps.
    auto IsSupporedByAPO = [&](){
        PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
        if (mediaCtx && !mediaCtx->m_apoDdiEnabled)
        return false;

        //Temporarily disable APO for CP, remove when switch to CP DDI APO
        if(profile == VAProfileProtected)
        {
            return false;
        }

        std::vector<VAEntrypoint> entrypointList(ctx->max_entrypoints);
        int32_t       entrypointNum   = -1;
        VAStatus vaStatus = VA_STATUS_SUCCESS;
        vaStatus = MediaLibvaInterfaceNext::QueryConfigEntrypoints(ctx, profile, entrypointList.data(), &entrypointNum);
        DDI_CHK_CONDITION((vaStatus != VA_STATUS_SUCCESS), "Invalid Profile",  false);
        auto tempEntrypoint = std::find(entrypointList.begin(), entrypointList.end(), entrypoint);
        DDI_CHK_CONDITION((tempEntrypoint == entrypointList.end()), "Invalid Entrypoint",  false);
        return true;
    };
    
    if (IsSupporedByAPO())
    {
        return MediaLibvaInterfaceNext::CreateConfig(ctx, profile, entrypoint, attrib_list, num_attribs, config_id);
    }
    else
    {
        return DdiMedia_CreateConfig(ctx, profile, entrypoint, attrib_list, num_attribs, config_id);
    }
}

VAStatus MediaLibvaInterface::DestroyConfig(
    VADriverContextP  ctx,
    VAConfigID        config_id)
{
    DDI_FUNCTION_ENTER();

   if (IS_VALID_CONFIG_ID(config_id))
   {
       return MediaLibvaInterfaceNext::DestroyConfig(ctx, config_id);
   }
   else
   {
       return DdiMedia_DestroyConfig(ctx, config_id);
   }
}

VAStatus MediaLibvaInterface::GetConfigAttributes(
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attrib_list,
    int32_t           num_attribs)
{
    DDI_FUNCTION_ENTER();

    VAStatus status = MediaLibvaInterfaceNext::GetConfigAttributes(ctx, profile, entrypoint, attrib_list, num_attribs);

    if(status != VA_STATUS_SUCCESS)
    {
        return DdiMedia_GetConfigAttributes(ctx, profile, entrypoint, attrib_list, num_attribs);
    }
    else
    {
        return status;
    }
} 

VAStatus MediaLibvaInterface::CreateContext(
    VADriverContextP  ctx,
    VAConfigID        config_id,
    int32_t           picture_width,
    int32_t           picture_height,
    int32_t           flag,
    VASurfaceID       *render_targets,
    int32_t           num_render_targets,
    VAContextID       *context)
{
    DDI_FUNCTION_ENTER();

    if(IS_VALID_CONFIG_ID(config_id))
    {
        return MediaLibvaInterfaceNext::CreateContext(ctx, config_id, picture_width, picture_height, flag,
            render_targets, num_render_targets, context);
    }
    else
    {
        return DdiMedia_CreateContext(ctx, config_id, picture_width, picture_height, flag,
            render_targets, num_render_targets, context);
    }
}

VAStatus MediaLibvaInterface::DestroyContext(
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_FUNCTION_ENTER();

    if(context > DDI_MEDIA_VACONTEXTID_BASE)
    {
        return MediaLibvaInterfaceNext::DestroyContext(ctx, context);
    }
    else
    {
        return DdiMedia_DestroyContext(ctx, context);
    }
}

VAStatus MediaLibvaInterface::CreateBuffer(
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferType      type,
    uint32_t          size,
    uint32_t          num_elements,
    void              *data,
    VABufferID        *bufId)
{
    DDI_FUNCTION_ENTER();

    if(context > DDI_MEDIA_VACONTEXTID_BASE)
    {
        return MediaLibvaInterfaceNext::CreateBuffer(ctx, context, type, size, num_elements, data, bufId);
    }
    else
    {
        return DdiMedia_CreateBuffer(ctx, context, type, size, num_elements, data, bufId);
    }
}

VAStatus MediaLibvaInterface::MapBuffer(
    VADriverContextP  ctx,
    VABufferID        buf_id,
    void              **pbuf)
{
    PDDI_MEDIA_CONTEXT mediaCtx = nullptr;
    uint32_t           ctxType  = DDI_MEDIA_CONTEXT_TYPE_NONE;
    DDI_FUNCTION_ENTER();

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, buf_id);
    // encoder/decoder need to add the cxtType condition to enable map buffer in APO path.
    if(mediaCtx->m_apoDdiEnabled && (ctxType == DDI_MEDIA_CONTEXT_TYPE_VP || ctxType == DDI_MEDIA_CONTEXT_TYPE_MEDIA || ctxType == DDI_MEDIA_CONTEXT_TYPE_ENCODER))
    {
        return MediaLibvaInterfaceNext::MapBuffer(ctx, buf_id, pbuf);
    }
    else
    {
        return DdiMedia_MapBuffer(ctx, buf_id, pbuf);
    }
}

VAStatus MediaLibvaInterface::UnmapBuffer(
    VADriverContextP  ctx,
    VABufferID        buf_id)
{
    PDDI_MEDIA_CONTEXT mediaCtx = nullptr;
    uint32_t           ctxType  = DDI_MEDIA_CONTEXT_TYPE_NONE;
    DDI_FUNCTION_ENTER();

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, buf_id);
    // encoder/decoder need to add the cxtType condition to enable map buffer in APO path.
    if(mediaCtx->m_apoDdiEnabled && (ctxType == DDI_MEDIA_CONTEXT_TYPE_VP || ctxType == DDI_MEDIA_CONTEXT_TYPE_MEDIA || ctxType == DDI_MEDIA_CONTEXT_TYPE_ENCODER))
    {
        return MediaLibvaInterfaceNext::UnmapBuffer(ctx, buf_id);
    }
    else
    {
        return DdiMedia_UnmapBuffer(ctx, buf_id);
    }
}

VAStatus MediaLibvaInterface::DestroyBuffer(
    VADriverContextP  ctx,
    VABufferID        buffer_id)
{
    PDDI_MEDIA_CONTEXT mediaCtx = nullptr;
    uint32_t           ctxType  = DDI_MEDIA_CONTEXT_TYPE_NONE;
    DDI_FUNCTION_ENTER();

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, buffer_id);

    // encoder/decoder need to add the cxtType condition to enable map buffer in APO path.
    if(mediaCtx->m_apoDdiEnabled && (ctxType == DDI_MEDIA_CONTEXT_TYPE_VP || ctxType == DDI_MEDIA_CONTEXT_TYPE_MEDIA || ctxType == DDI_MEDIA_CONTEXT_TYPE_ENCODER))
    {
        return MediaLibvaInterfaceNext::DestroyBuffer(ctx, buffer_id);
    }
    else
    {
        return DdiMedia_DestroyBuffer(ctx, buffer_id);
    }
}

VAStatus MediaLibvaInterface::BeginPicture(
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       render_target)
{
    DDI_FUNCTION_ENTER();

    if(context > DDI_MEDIA_VACONTEXTID_BASE)
    {
        return MediaLibvaInterfaceNext::BeginPicture(ctx, context, render_target);
    }
    else
    {
        return DdiMedia_BeginPicture(ctx, context, render_target);
    }

}

VAStatus MediaLibvaInterface::RenderPicture(
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           num_buffers)
{
    DDI_FUNCTION_ENTER();
    if(context > DDI_MEDIA_VACONTEXTID_BASE)
    {
        return MediaLibvaInterfaceNext::RenderPicture(ctx, context, buffers, num_buffers);
    }
    else
    {
        return DdiMedia_RenderPicture(ctx, context, buffers, num_buffers);
    }
}

VAStatus MediaLibvaInterface::EndPicture(
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_FUNCTION_ENTER();

    if(context > DDI_MEDIA_VACONTEXTID_BASE)
    {
        return MediaLibvaInterfaceNext::EndPicture(ctx, context);
    }
    else
    {
        return DdiMedia_EndPicture(ctx, context);
    }
}

VAStatus MediaLibvaInterface::SyncSurface(
    VADriverContextP  ctx,
    VASurfaceID       render_target)
{
    PDDI_MEDIA_CONTEXT mediaCtx    = nullptr;
    DDI_MEDIA_SURFACE  *ddiSurface = nullptr;
    DDI_FUNCTION_ENTER();

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    ddiSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, render_target);
    DDI_CHK_NULL(ddiSurface, "nullptr ddiSurface", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(mediaCtx->m_apoDdiEnabled && ddiSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        return MediaLibvaInterfaceNext::SyncSurface(ctx, render_target);
    }
    else
    {
        return DdiMedia_SyncSurface(ctx, render_target);
    }
}

#if VA_CHECK_VERSION(1, 9, 0)

VAStatus MediaLibvaInterface::SyncSurface2(
    VADriverContextP  ctx,
    VASurfaceID       surface_id,
    uint64_t          timeout_ns)
{
    PDDI_MEDIA_CONTEXT mediaCtx    = nullptr;
    DDI_MEDIA_SURFACE  *ddiSurface = nullptr;
    DDI_FUNCTION_ENTER();

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    ddiSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface_id);
    DDI_CHK_NULL(ddiSurface, "nullptr ddiSurface", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(mediaCtx->m_apoDdiEnabled && ddiSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        return MediaLibvaInterfaceNext::SyncSurface2(ctx, surface_id, timeout_ns);
    }
    else
    {
        return DdiMedia_SyncSurface2(ctx, surface_id, timeout_ns);
    }
}
#endif

VAStatus MediaLibvaInterface::QuerySurfaceError(
    VADriverContextP  ctx,
    VASurfaceID       render_target,
    VAStatus          error_status,
    void              **error_info)
{
    return DdiMedia_QuerySurfaceError(
        ctx, render_target, error_status, error_info);
}

VAStatus MediaLibvaInterface::QuerySurfaceAttributes(
    VADriverContextP  ctx,
    VAConfigID        config_id,
    VASurfaceAttrib   *attrib_list,
    uint32_t          *num_attribs)
{
    DDI_FUNCTION_ENTER();

    if (IS_VALID_CONFIG_ID(config_id))
    {
        return MediaLibvaInterfaceNext::QuerySurfaceAttributes(
            ctx, config_id, attrib_list, num_attribs);
    }
    else
    {
        return DdiMedia_QuerySurfaceAttributes(
            ctx, config_id, attrib_list, num_attribs);
    }
}

VAStatus MediaLibvaInterface::PutSurface(
    VADriverContextP  ctx,
    VASurfaceID       surface,
    void*             draw,
    int16_t           srcx,
    int16_t           srcy,
    uint16_t          srcw,
    uint16_t          srch,
    int16_t           destx,
    int16_t           desty,
    uint16_t          destw,
    uint16_t          desth,
    VARectangle       *cliprects,
    uint32_t          number_cliprects,
    uint32_t          flags)
{
    PDDI_MEDIA_CONTEXT mediaCtx    = nullptr;
    DDI_MEDIA_SURFACE  *ddiSurface = nullptr;
    DDI_FUNCTION_ENTER();

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    ddiSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(ddiSurface, "nullptr ddiSurface", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(mediaCtx->m_apoDdiEnabled && ddiSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        return MediaLibvaInterfaceNext::PutSurface(ctx, surface, draw, srcx, srcy, srcw, srch, destx,
            desty, destw, desth, cliprects, number_cliprects, flags);
    }
    else
    {
        return DdiMedia_PutSurface(ctx, surface, draw, srcx, srcy, srcw, srch, destx,
            desty, destw, desth, cliprects, number_cliprects, flags);
    }
}

VAStatus MediaLibvaInterface::QueryImageFormats(
    VADriverContextP  ctx,
    VAImageFormat     *format_list,
    int32_t           *num_formats)
{
    VAStatus status = MediaLibvaInterfaceNext::QueryImageFormats(
        ctx, format_list, num_formats);

    if(status != VA_STATUS_SUCCESS)
    {
        return DdiMedia_QueryImageFormats(ctx, format_list, num_formats);
    }
    else
    {
        return status;
    }
}

VAStatus MediaLibvaInterface::GetImage(
    VADriverContextP  ctx,
    VASurfaceID       surface,
    int32_t           x,
    int32_t           y,
    uint32_t          width,
    uint32_t          height,
    VAImageID         image)
{
    PDDI_MEDIA_CONTEXT mediaCtx    = nullptr;
    DDI_MEDIA_SURFACE  *ddiSurface = nullptr;
    DDI_FUNCTION_ENTER();

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    ddiSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(ddiSurface, "nullptr ddiSurface", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(mediaCtx->m_apoDdiEnabled && ddiSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        return MediaLibvaInterfaceNext::GetImage(ctx, surface, x, y, width, height, image);
    }
    else
    {
        return DdiMedia_GetImage(ctx, surface, x, y, width, height, image);
    }
}

VAStatus MediaLibvaInterface::PutImage(
    VADriverContextP  ctx,
    VASurfaceID       surface,
    VAImageID         image,
    int32_t           src_x,
    int32_t           src_y,
    uint32_t          src_width,
    uint32_t          src_height,
    int32_t           dest_x,
    int32_t           dest_y,
    uint32_t          dest_width,
    uint32_t          dest_height)
{
    PDDI_MEDIA_CONTEXT mediaCtx = nullptr;
    DDI_MEDIA_SURFACE  *ddiSurface = nullptr;
    DDI_FUNCTION_ENTER();

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    ddiSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(ddiSurface, "nullptr ddiSurface", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(mediaCtx->m_apoDdiEnabled && ddiSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        return MediaLibvaInterfaceNext::PutImage(ctx, surface, image, src_x, src_y, src_width, src_height,
            dest_x, dest_y, dest_width, dest_height);
    }
    else
    {
        return DdiMedia_PutImage(ctx, surface, image, src_x, src_y, src_width, src_height,
            dest_x, dest_y, dest_width, dest_height);
    }
}

VAStatus MediaLibvaInterface::QueryDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute  *attr_list,
    int32_t             *num_attributes)
{
    return DdiMedia_QueryDisplayAttributes(ctx, attr_list, num_attributes);
}

VAStatus MediaLibvaInterface::GetDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute  *attr_list,
    int32_t             num_attributes)
{
    return DdiMedia_GetDisplayAttributes(ctx, attr_list, num_attributes);
}

VAStatus MediaLibvaInterface::QueryProcessingRate(
    VADriverContextP           ctx,
    VAConfigID                 config_id,
    VAProcessingRateParameter  *proc_buf,
    uint32_t                   *processing_rate)
{
    return DdiMedia_QueryProcessingRate(ctx, config_id, proc_buf, processing_rate);
}

#if VA_CHECK_VERSION(1,10,0)
VAStatus MediaLibvaInterface::Copy(
    VADriverContextP  ctx,
    VACopyObject      *dst_obj,
    VACopyObject      *src_obj,
    VACopyOption      option)
{
    return DdiMedia_Copy(ctx, dst_obj, src_obj, option);
}
#endif

VAStatus MediaLibvaInterface::BufferInfo(
    VADriverContextP  ctx,
    VABufferID        buf_id,
    VABufferType      *type,
    uint32_t          *size,
    uint32_t          *num_elements)
{
    return DdiMedia_BufferInfo(ctx, buf_id, type, size, num_elements);
}

VAStatus MediaLibvaInterface::LockSurface(
    VADriverContextP  ctx,
    VASurfaceID       surface,
    uint32_t          *fourcc,
    uint32_t          *luma_stride,
    uint32_t          *chroma_u_stride,
    uint32_t          *chroma_v_stride,
    uint32_t          *luma_offset,
    uint32_t          *chroma_u_offset,
    uint32_t          *chroma_v_offset,
    uint32_t          *buffer_name,
    void              **buffer)
{
    return DdiMedia_LockSurface(ctx, surface, fourcc, luma_stride, chroma_u_stride, 
        chroma_v_stride, luma_offset, chroma_u_offset, chroma_v_offset, buffer_name, buffer);
}

VAStatus MediaLibvaInterface::UnlockSurface(
    VADriverContextP  ctx,
    VASurfaceID       surface)
{
    return DdiMedia_UnlockSurface(ctx, surface);
}

VAStatus MediaLibvaInterface::QueryVideoProcFilters(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  *filters,
    uint32_t          *num_filters)
{
    DDI_FUNCTION_ENTER();

    if(context > DDI_MEDIA_VACONTEXTID_BASE)
    {
        return MediaLibvaInterfaceNext::QueryVideoProcFilters(ctx, context, filters, num_filters);
    }
    else
    {
        return DdiMedia_QueryVideoProcFilters(ctx, context, filters, num_filters);
    }
}

VAStatus MediaLibvaInterface::QueryVideoProcFilterCaps(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  type,
    void              *filter_caps,
    uint32_t          *num_filter_caps)
{
    DDI_FUNCTION_ENTER();

    if(context > DDI_MEDIA_VACONTEXTID_BASE)
    {
        return MediaLibvaInterfaceNext::QueryVideoProcFilterCaps(ctx, context, type, filter_caps, num_filter_caps);
    }
    else
    {
        return DdiMedia_QueryVideoProcFilterCaps(ctx, context, type, filter_caps, num_filter_caps);
    }
}

VAStatus MediaLibvaInterface::QueryVideoProcPipelineCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID          *filters,
    uint32_t            num_filters,
    VAProcPipelineCaps  *pipeline_caps)
{
    DDI_FUNCTION_ENTER();

    if(context > DDI_MEDIA_VACONTEXTID_BASE)
    {
        return MediaLibvaInterfaceNext::QueryVideoProcPipelineCaps(ctx, context, filters, num_filters,
            pipeline_caps);
    }
    else
    {
        return DdiMedia_QueryVideoProcPipelineCaps(ctx, context, filters, num_filters,
            pipeline_caps);
    }
}

#if VA_CHECK_VERSION(1,11,0)
VAStatus MediaLibvaInterface::CreateProtectedSession(
    VADriverContextP      ctx,
    VAConfigID            config_id,
    VAProtectedSessionID  *protected_session)
{
    return DdiMediaProtected::DdiMedia_CreateProtectedSession(ctx, config_id, protected_session);
}

VAStatus MediaLibvaInterface::DestroyProtectedSession(
    VADriverContextP      ctx,
    VAProtectedSessionID  protected_session)
{
    return DdiMediaProtected::DdiMedia_DestroyProtectedSession(ctx, protected_session);
}

VAStatus MediaLibvaInterface::AttachProtectedSession(
    VADriverContextP      ctx,
    VAContextID           context,
    VAProtectedSessionID  protected_session)
{
    return DdiMediaProtected::DdiMedia_AttachProtectedSession(ctx, context, protected_session);
}

VAStatus MediaLibvaInterface::DetachProtectedSession(
    VADriverContextP  ctx,
    VAContextID       context)
{
    return DdiMediaProtected::DdiMedia_DetachProtectedSession(ctx, context);
}

VAStatus MediaLibvaInterface::ProtectedSessionExecute(
    VADriverContextP      ctx,
    VAProtectedSessionID  protected_session,
    VABufferID            data)
{
    return DdiMediaProtected::DdiMedia_ProtectedSessionExecute(ctx, protected_session, data);
}
#endif

VAStatus MediaLibvaInterface::GetSurfaceAttributes(
    VADriverContextP  ctx,
    VAConfigID        config,
    VASurfaceAttrib   *attrib_list,
    uint32_t          num_attribs)
{
    return DdiMedia_GetSurfaceAttributes(ctx, config, attrib_list, num_attribs);
}

VAStatus MediaLibvaInterface::AcquireBufferHandle(
    VADriverContextP  ctx,
    VABufferID        buf_id,
    VABufferInfo      *buf_info)
{
    return DdiMedia_AcquireBufferHandle(ctx, buf_id, buf_info);
}

VAStatus MediaLibvaInterface::ReleaseBufferHandle(
    VADriverContextP  ctx,
    VABufferID        buf_id)
{
    return DdiMedia_ReleaseBufferHandle(ctx, buf_id);
}

VAStatus MediaLibvaInterface::ExportSurfaceHandle(
    VADriverContextP  ctx,
    VASurfaceID       surface_id,
    uint32_t          mem_type,
    uint32_t          flags,
    void              *descriptor)
{
    return DdiMedia_ExportSurfaceHandle(ctx, surface_id, mem_type, flags, descriptor);
}

#ifndef ANDROID
VAStatus MediaLibvaInterface::CreateMfeContextInternal(
    VADriverContextP  ctx,
    VAMFContextID     *mfe_context)
{
    return DdiMedia_CreateMfeContextInternal(ctx, mfe_context);
}

VAStatus MediaLibvaInterface::AddContextInternal(
    VADriverContextP  ctx,
    VAContextID       context,
    VAMFContextID     mfe_context)
{
    return DdiMedia_AddContextInternal(ctx, context, mfe_context);
}

VAStatus MediaLibvaInterface::ReleaseContextInternal(
    VADriverContextP  ctx,
    VAContextID       context,
    VAMFContextID     mfe_context)
{
    return DdiMedia_ReleaseContextInternal(ctx, context, mfe_context);
}

VAStatus MediaLibvaInterface::MfeSubmit(
    VADriverContextP  ctx,
    VAMFContextID     mfe_context,
    VAContextID       *contexts,
    int32_t           num_contexts)
{
    return DdiEncode_MfeSubmit(ctx, mfe_context, contexts, num_contexts);
}

#endif