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
    pVTable->vaQueryConfigEntrypoints        = MediaLibvaInterfaceNext::QueryConfigEntrypoints;
    pVTable->vaQueryConfigProfiles           = MediaLibvaInterfaceNext::QueryConfigProfiles;
    pVTable->vaQueryConfigAttributes         = QueryConfigAttributes;
    pVTable->vaCreateConfig                  = CreateConfig;
    pVTable->vaDestroyConfig                 = MediaLibvaInterfaceNext::DestroyConfig;
    pVTable->vaGetConfigAttributes           = MediaLibvaInterfaceNext::GetConfigAttributes;

    pVTable->vaCreateSurfaces                = MediaLibvaInterfaceNext::CreateSurfaces;
    pVTable->vaDestroySurfaces               = MediaLibvaInterfaceNext::DestroySurfaces;
    pVTable->vaCreateSurfaces2               = MediaLibvaInterfaceNext::CreateSurfaces2;

    pVTable->vaCreateContext                 = MediaLibvaInterfaceNext::CreateContext;
    pVTable->vaDestroyContext                = MediaLibvaInterfaceNext::DestroyContext;
    pVTable->vaCreateBuffer                  = MediaLibvaInterfaceNext::CreateBuffer;
    pVTable->vaBufferSetNumElements          = MediaLibvaInterfaceNext::BufferSetNumElements;
    pVTable->vaMapBuffer                     = MapBuffer;
    pVTable->vaUnmapBuffer                   = UnmapBuffer;
    pVTable->vaDestroyBuffer                 = DestroyBuffer;
    pVTable->vaBeginPicture                  = MediaLibvaInterfaceNext::BeginPicture;
    pVTable->vaRenderPicture                 = MediaLibvaInterfaceNext::RenderPicture;
    pVTable->vaEndPicture                    = MediaLibvaInterfaceNext::EndPicture;
    pVTable->vaSyncSurface                   = SyncSurface;
#if VA_CHECK_VERSION(1, 9, 0)
    pVTable->vaSyncSurface2                  = SyncSurface2;
    pVTable->vaSyncBuffer                    = MediaLibvaInterfaceNext::SyncBuffer;
#endif
    pVTable->vaQuerySurfaceStatus            = MediaLibvaInterfaceNext::QuerySurfaceStatus;
    pVTable->vaQuerySurfaceError             = QuerySurfaceError;
    pVTable->vaQuerySurfaceAttributes        = MediaLibvaInterfaceNext::QuerySurfaceAttributes;
    pVTable->vaPutSurface                    = PutSurface;
    pVTable->vaQueryImageFormats             = MediaLibvaInterfaceNext::QueryImageFormats;

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
    pVTable->vaLockSurface                   = MediaLibvaInterfaceNext::LockSurface;
    pVTable->vaUnlockSurface                 = MediaLibvaInterfaceNext::UnlockSurface;

    pVTableVpp->vaQueryVideoProcFilters      = MediaLibvaInterfaceNext::QueryVideoProcFilters;
    pVTableVpp->vaQueryVideoProcFilterCaps   = MediaLibvaInterfaceNext::QueryVideoProcFilterCaps;
    pVTableVpp->vaQueryVideoProcPipelineCaps = MediaLibvaInterfaceNext::QueryVideoProcPipelineCaps;

#if VA_CHECK_VERSION(1,11,0)
    pVTableProt->vaCreateProtectedSession    = MediaLibvaInterfaceNext::CreateProtectedSession;
    pVTableProt->vaDestroyProtectedSession   = MediaLibvaInterfaceNext::DestroyProtectedSession;
    pVTableProt->vaAttachProtectedSession    = MediaLibvaInterfaceNext::AttachProtectedSession;
    pVTableProt->vaDetachProtectedSession    = MediaLibvaInterfaceNext::DetachProtectedSession;
    pVTableProt->vaProtectedSessionExecute   = MediaLibvaInterfaceNext::ProtectedSessionExecute;
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
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    auto IsSupportedByAPO = [&](){
        if (!mediaCtx->m_apoDdiEnabled)
        return false;

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

    return MediaLibvaInterfaceNext::QueryConfigAttributes(ctx, config_id, profile, entrypoint, attrib_list, num_attribs);
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

        std::vector<VAEntrypoint> entrypointList(ctx->max_entrypoints);
        int32_t       entrypointNum   = -1;
        VAStatus vaStatus = VA_STATUS_SUCCESS;
        vaStatus = MediaLibvaInterfaceNext::QueryConfigEntrypoints(ctx, profile, entrypointList.data(), &entrypointNum);
        DDI_CHK_CONDITION((vaStatus != VA_STATUS_SUCCESS), "Invalid Profile",  false);
        auto tempEntrypoint = std::find(entrypointList.begin(), entrypointList.end(), entrypoint);
        DDI_CHK_CONDITION((tempEntrypoint == entrypointList.end()), "Invalid Entrypoint",  false);
        return true;
    };
    
    return MediaLibvaInterfaceNext::CreateConfig(ctx, profile, entrypoint, attrib_list, num_attribs, config_id);
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

    return MediaLibvaInterfaceNext::MapBuffer(ctx, buf_id, pbuf);
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

    return MediaLibvaInterfaceNext::UnmapBuffer(ctx, buf_id);
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

    return MediaLibvaInterfaceNext::DestroyBuffer(ctx, buffer_id);
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

    return MediaLibvaInterfaceNext::SyncSurface(ctx, render_target);
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

    return MediaLibvaInterfaceNext::SyncSurface2(ctx, surface_id, timeout_ns);
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

    return MediaLibvaInterfaceNext::PutSurface(ctx, surface, draw, srcx, srcy, srcw, srch, destx,
                                               desty, destw, desth, cliprects, number_cliprects, flags);
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

    return MediaLibvaInterfaceNext::GetImage(ctx, surface, x, y, width, height, image);
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

    return MediaLibvaInterfaceNext::PutImage(ctx, surface, image, src_x, src_y, src_width, src_height,
                                             dest_x, dest_y, dest_width, dest_height);
}

VAStatus MediaLibvaInterface::QueryDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute  *attr_list,
    int32_t             *num_attributes)
{
    DDI_FUNCTION_ENTER();

    return MediaLibvaInterfaceNext::QueryDisplayAttributes(ctx, attr_list, num_attributes);
}

VAStatus MediaLibvaInterface::GetDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute  *attr_list,
    int32_t             num_attributes)
{
    DDI_FUNCTION_ENTER();

    return MediaLibvaInterfaceNext::GetDisplayAttributes(ctx, attr_list, num_attributes);
}

VAStatus MediaLibvaInterface::QueryProcessingRate(
    VADriverContextP           ctx,
    VAConfigID                 config_id,
    VAProcessingRateParameter  *proc_buf,
    uint32_t                   *processing_rate)
{
    DDI_FUNCTION_ENTER();

    return DdiMedia_QueryProcessingRate(ctx, config_id, proc_buf, processing_rate);
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
    DDI_FUNCTION_ENTER();

    return DdiMedia_GetSurfaceAttributes(ctx, config, attrib_list, num_attribs);
}

#ifndef ANDROID
VAStatus MediaLibvaInterface::CreateMfeContextInternal(
    VADriverContextP  ctx,
    VAMFContextID     *mfe_context)
{
    DDI_FUNCTION_ENTER();

    return DdiMedia_CreateMfeContextInternal(ctx, mfe_context);
}

VAStatus MediaLibvaInterface::AddContextInternal(
    VADriverContextP  ctx,
    VAContextID       context,
    VAMFContextID     mfe_context)
{
    DDI_FUNCTION_ENTER();

    return DdiMedia_AddContextInternal(ctx, context, mfe_context);
}

VAStatus MediaLibvaInterface::ReleaseContextInternal(
    VADriverContextP  ctx,
    VAContextID       context,
    VAMFContextID     mfe_context)
{
    DDI_FUNCTION_ENTER();

    return DdiMedia_ReleaseContextInternal(ctx, context, mfe_context);
}

VAStatus MediaLibvaInterface::MfeSubmit(
    VADriverContextP  ctx,
    VAMFContextID     mfe_context,
    VAContextID       *contexts,
    int32_t           num_contexts)
{
    DDI_FUNCTION_ENTER();

    return DdiEncode_MfeSubmit(ctx, mfe_context, contexts, num_contexts);
}

#endif
