/*
* Copyright (c) 2021, Intel Corporation
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

#include "media_libva_interface_next.h"

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

    ctx->pDriverData                         = nullptr;
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

    pVTable->vaCreateSurfaces                = CreateSurfaces;
    pVTable->vaDestroySurfaces               = DestroySurfaces;
    pVTable->vaCreateSurfaces2               = CreateSurfaces2;

    pVTable->vaCreateContext                 = CreateContext;
    pVTable->vaDestroyContext                = DestroyContext;
    pVTable->vaCreateBuffer                  = CreateBuffer;
    pVTable->vaBufferSetNumElements          = BufferSetNumElements;
    pVTable->vaMapBuffer                     = MapBuffer;
    pVTable->vaUnmapBuffer                   = UnmapBuffer;
    pVTable->vaDestroyBuffer                 = DestroyBuffer;
    pVTable->vaBeginPicture                  = BeginPicture;
    pVTable->vaRenderPicture                 = RenderPicture;
    pVTable->vaEndPicture                    = EndPicture;
    pVTable->vaSyncSurface                   = SyncSurface;
#if VA_CHECK_VERSION(1, 9, 0)
    pVTable->vaSyncSurface2                  = SyncSurface2;
    pVTable->vaSyncBuffer                    = SyncBuffer;
#endif
    pVTable->vaQuerySurfaceStatus            = QuerySurfaceStatus;
    pVTable->vaQuerySurfaceError             = QuerySurfaceError;
    pVTable->vaQuerySurfaceAttributes        = QuerySurfaceAttributes;
    pVTable->vaPutSurface                    = PutSurface;
    pVTable->vaQueryImageFormats             = QueryImageFormats;

    pVTable->vaCreateImage                   = CreateImage;
    pVTable->vaDeriveImage                   = DeriveImage;
    pVTable->vaDestroyImage                  = DestroyImage;
    pVTable->vaSetImagePalette               = SetImagePalette;
    pVTable->vaGetImage                      = GetImage;
    pVTable->vaPutImage                      = PutImage;
    pVTable->vaQuerySubpictureFormats        = QuerySubpictureFormats;
    pVTable->vaCreateSubpicture              = CreateSubpicture;
    pVTable->vaDestroySubpicture             = DestroySubpicture;
    pVTable->vaSetSubpictureImage            = SetSubpictureImage;
    pVTable->vaSetSubpictureChromakey        = SetSubpictureChromakey;
    pVTable->vaSetSubpictureGlobalAlpha      = SetSubpictureGlobalAlpha;
    pVTable->vaAssociateSubpicture           = AssociateSubpicture;
    pVTable->vaDeassociateSubpicture         = DeassociateSubpicture;
    pVTable->vaQueryDisplayAttributes        = QueryDisplayAttributes;
    pVTable->vaGetDisplayAttributes          = GetDisplayAttributes;
    pVTable->vaSetDisplayAttributes          = SetDisplayAttributes;
    pVTable->vaQueryProcessingRate           = QueryProcessingRate;
#if VA_CHECK_VERSION(1,10,0)
    pVTable->vaCopy                          = Copy;
#endif

    // vaTrace
    pVTable->vaBufferInfo                    = BufferInfo;
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
    pVTable->vaGetSurfaceAttributes          = GetSurfaceAttributes;
    //Export PRIMEFD/FLINK to application for buffer sharing with OpenCL/GL
    pVTable->vaAcquireBufferHandle           = AcquireBufferHandle;
    pVTable->vaReleaseBufferHandle           = ReleaseBufferHandle;
    pVTable->vaExportSurfaceHandle           = ExportSurfaceHandle;
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
    VAStatus status = MediaLibvaInterfaceNext::QueryConfigEntrypoints(
        ctx, profile, entrypoint_list, num_entrypoints);

    if (status != VA_STATUS_SUCCESS)
    {
        return DdiMedia_QueryConfigEntrypoints(ctx, profile, entrypoint_list,
            num_entrypoints);
    }
    else
    {
        return status;
    }
}

VAStatus MediaLibvaInterface::QueryConfigProfiles(
    VADriverContextP  ctx,
    VAProfile         *profile_list,
    int32_t           *num_profiles)
{
    VAStatus status = MediaLibvaInterfaceNext::QueryConfigProfiles(
        ctx, profile_list, num_profiles);

    if(status != VA_STATUS_SUCCESS)
    {
        return DdiMedia_QueryConfigProfiles(ctx, profile_list, num_profiles);
    }
    else
    {
        return status;
    }
}

VAStatus MediaLibvaInterface::QueryConfigAttributes(
    VADriverContextP  ctx,
    VAConfigID        config_id,
    VAProfile         *profile,
    VAEntrypoint      *entrypoint,
    VAConfigAttrib    *attrib_list,
    int32_t           *num_attribs)
{
    return DdiMedia_QueryConfigAttributes(ctx, config_id, profile, entrypoint,
        attrib_list, num_attribs);
}

VAStatus MediaLibvaInterface::CreateConfig(
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attrib_list,
    int32_t           num_attribs,
    VAConfigID        *config_id)
{
    return DdiMedia_CreateConfig(ctx, profile, entrypoint, attrib_list, num_attribs,
        config_id);
}

VAStatus MediaLibvaInterface::DestroyConfig(
    VADriverContextP  ctx,
    VAConfigID        config_id)
{
    return DdiMedia_DestroyConfig(ctx, config_id);
}

VAStatus MediaLibvaInterface::GetConfigAttributes(
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attrib_list,
    int32_t           num_attribs)
{
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

VAStatus MediaLibvaInterface::CreateSurfaces(
    VADriverContextP  ctx,
    int32_t           width,
    int32_t           height,
    int32_t           format,
    int32_t           num_surfaces,
    VASurfaceID       *surfaces)
{
    return DdiMedia_CreateSurfaces(ctx, width, height, format, num_surfaces, surfaces);
}

VAStatus MediaLibvaInterface::DestroySurfaces(
    VADriverContextP  ctx,
    VASurfaceID       *surfaces,
    int32_t           num_surfaces)
{
    return DdiMedia_DestroySurfaces(ctx, surfaces, num_surfaces);
}

VAStatus MediaLibvaInterface::CreateSurfaces2(
    VADriverContextP  ctx,
    uint32_t          format,
    uint32_t          width,
    uint32_t          height,
    VASurfaceID       *surfaces,
    uint32_t          num_surfaces,
    VASurfaceAttrib   *attrib_list,
    uint32_t          num_attribs)
{
    return DdiMedia_CreateSurfaces2(ctx, format, width, height, surfaces, num_surfaces, 
        attrib_list, num_attribs);
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
    return DdiMedia_CreateContext(ctx, config_id, picture_width, picture_height, flag,
        render_targets, num_render_targets, context);
}

VAStatus MediaLibvaInterface::DestroyContext(
    VADriverContextP  ctx,
    VAContextID       context)
{
    return DdiMedia_DestroyContext(ctx, context);
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
    return DdiMedia_CreateBuffer(ctx, context, type, size, num_elements, data, bufId);
}

VAStatus MediaLibvaInterface::BufferSetNumElements(
    VADriverContextP  ctx,
    VABufferID        buf_id,
    uint32_t          num_elements)
{
    return DdiMedia_BufferSetNumElements(ctx, buf_id, num_elements);
}

VAStatus MediaLibvaInterface::MapBuffer(
    VADriverContextP  ctx,
    VABufferID        buf_id,
    void              **pbuf)
{
    return DdiMedia_MapBuffer(ctx, buf_id, pbuf);
}

VAStatus MediaLibvaInterface::UnmapBuffer(
    VADriverContextP  ctx,
    VABufferID        buf_id)
{
    return DdiMedia_UnmapBuffer(ctx, buf_id);
}

VAStatus MediaLibvaInterface::DestroyBuffer(
    VADriverContextP  ctx,
    VABufferID        buffer_id)
{
    return DdiMedia_DestroyBuffer(ctx, buffer_id);
}

VAStatus MediaLibvaInterface::BeginPicture(
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       render_target)
{
    return DdiMedia_BeginPicture(ctx, context, render_target);
}

VAStatus MediaLibvaInterface::RenderPicture(
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           num_buffers)
{
    return DdiMedia_RenderPicture(ctx, context, buffers, num_buffers);
}

VAStatus MediaLibvaInterface::EndPicture(
    VADriverContextP  ctx,
    VAContextID       context)
{
    return DdiMedia_EndPicture(ctx, context);
}

VAStatus MediaLibvaInterface::SyncSurface(
    VADriverContextP  ctx,
    VASurfaceID       render_target)
{
    return DdiMedia_SyncSurface(ctx, render_target);
}

#if VA_CHECK_VERSION(1, 9, 0)

VAStatus MediaLibvaInterface::SyncSurface2(
    VADriverContextP  ctx,
    VASurfaceID       surface_id,
    uint64_t          timeout_ns)
{
    return DdiMedia_SyncSurface2(ctx, surface_id, timeout_ns);
}


VAStatus MediaLibvaInterface::SyncBuffer(
    VADriverContextP  ctx,
    VABufferID        buf_id,
    uint64_t          timeout_ns)
{
    return DdiMedia_SyncBuffer(ctx, buf_id, timeout_ns);
}
#endif

VAStatus MediaLibvaInterface::QuerySurfaceStatus(
    VADriverContextP  ctx,
    VASurfaceID       render_target,
    VASurfaceStatus   *status)
{
    return DdiMedia_QuerySurfaceStatus(ctx, render_target, status);
}

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
    return DdiMedia_QuerySurfaceAttributes(
        ctx, config_id, attrib_list, num_attribs);
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
    return DdiMedia_PutSurface(ctx, surface, draw, srcx, srcy, srcw, srch, destx, 
        desty, destw, desth, cliprects, number_cliprects, flags);
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

VAStatus MediaLibvaInterface::CreateImage(
    VADriverContextP  ctx,
    VAImageFormat     *format,
    int32_t           width,
    int32_t           height,
    VAImage           *image)
{
    return DdiMedia_CreateImage(ctx, format, width, height, image);
}

VAStatus MediaLibvaInterface::DeriveImage(
    VADriverContextP  ctx,
    VASurfaceID       surface,
    VAImage           *image)
{
    return DdiMedia_DeriveImage(ctx, surface, image);
}

VAStatus MediaLibvaInterface::DestroyImage(
    VADriverContextP  ctx,
    VAImageID         image)
{
    return DdiMedia_DestroyImage(ctx, image);
}

VAStatus MediaLibvaInterface::SetImagePalette(
    VADriverContextP  ctx,
    VAImageID         image,
    unsigned char     *palette)
{
    return DdiMedia_SetImagePalette(ctx, image, palette);
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
    return DdiMedia_GetImage(ctx, surface, x, y, width, height, image);
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
    return DdiMedia_PutImage(ctx, surface, image, src_x, src_y, src_width, src_height,
        dest_x, dest_y, dest_width, dest_height);
}

VAStatus MediaLibvaInterface::QuerySubpictureFormats(
    VADriverContextP  ctx,
    VAImageFormat     *format_list,
    uint32_t          *flags,
    uint32_t          *num_formats)
{
    return DdiMedia_QuerySubpictureFormats(ctx, format_list, flags, num_formats);
}

VAStatus MediaLibvaInterface::CreateSubpicture(
    VADriverContextP  ctx,
    VAImageID         image,
    VASubpictureID    *subpicture)
{
    return DdiMedia_CreateSubpicture(ctx, image, subpicture);
}

VAStatus MediaLibvaInterface::DestroySubpicture(
    VADriverContextP  ctx,
    VASubpictureID    subpicture)
{
    return DdiMedia_DestroySubpicture(ctx, subpicture);
}

VAStatus MediaLibvaInterface::SetSubpictureImage(
    VADriverContextP  ctx,
    VASubpictureID    subpicture,
    VAImageID         image)
{
    return DdiMedia_SetSubpictureImage(ctx, subpicture, image);
}

VAStatus MediaLibvaInterface::SetSubpictureChromakey(
    VADriverContextP  ctx,
    VASubpictureID    subpicture,
    uint32_t          chromakey_min,
    uint32_t          chromakey_max,
    uint32_t          chromakey_mask)
{
    return DdiMedia_SetSubpictureChromakey(ctx, subpicture, chromakey_min, chromakey_max,
        chromakey_mask);
}

VAStatus MediaLibvaInterface::SetSubpictureGlobalAlpha(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    float            global_alpha)
{
    return DdiMedia_SetSubpictureGlobalAlpha(ctx, subpicture, global_alpha);
}

VAStatus MediaLibvaInterface::AssociateSubpicture(
    VADriverContextP  ctx,
    VASubpictureID    subpicture,
    VASurfaceID       *target_surfaces,
    int32_t           num_surfaces,
    int16_t           src_x,
    int16_t           src_y,
    uint16_t          src_width,
    uint16_t          src_height,
    int16_t           dest_x,
    int16_t           dest_y,
    uint16_t          dest_width,
    uint16_t          dest_height,
    uint32_t          flags)
{
    return DdiMedia_AssociateSubpicture(ctx, subpicture, target_surfaces, num_surfaces,
        src_x, src_y, src_width, src_height, dest_x, dest_y, dest_width, dest_height, flags);
}

VAStatus MediaLibvaInterface::DeassociateSubpicture(
    VADriverContextP  ctx,
    VASubpictureID    subpicture,
    VASurfaceID       *target_surfaces,
    int32_t           num_surfaces)
{
    return DdiMedia_DeassociateSubpicture(ctx, subpicture, target_surfaces, num_surfaces);
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

VAStatus MediaLibvaInterface::SetDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute  *attr_list,
    int32_t             num_attributes)
{
    return DdiMedia_SetDisplayAttributes(ctx, attr_list, num_attributes);
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
    return DdiMedia_QueryVideoProcFilters(ctx, context, filters, num_filters);
}

VAStatus MediaLibvaInterface::QueryVideoProcFilterCaps(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  type,
    void              *filter_caps,
    uint32_t          *num_filter_caps)
{
    return DdiMedia_QueryVideoProcFilterCaps(ctx, context, type, filter_caps, num_filter_caps);
}

VAStatus MediaLibvaInterface::QueryVideoProcPipelineCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID          *filters,
    uint32_t            num_filters,
    VAProcPipelineCaps  *pipeline_caps)
{
    return DdiMedia_QueryVideoProcPipelineCaps(ctx, context, filters, num_filters, 
        pipeline_caps);
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