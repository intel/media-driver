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
//! \file     ddi_decode_functions.cpp
//! \brief    ddi decode functions implementaion.
//!

#include "ddi_decode_functions.h"
#include "media_libva_util_next.h"
#include "media_libva_common_next.h"

#define DDI_DECODE_SFC_MAX_WIDTH                   4096
#define DDI_DECODE_SFC_MAX_HEIGHT                  4096
#define DDI_DECODE_SFC_MIN_WIDTH                   128
#define DDI_DECODE_SFC_MIN_HEIGHT                  128
#define DDI_DECODE_HCP_SFC_MAX_WIDTH               (16*1024)
#define DDI_DECODE_HCP_SFC_MAX_HEIGHT              (16*1024)

VAStatus DdiDecodeFunctions::CreateContext (
    VADriverContextP  ctx,
    VAConfigID        configId,
    int32_t           pictureWidth,
    int32_t           pictureHeight,
    int32_t           flag,
    VASurfaceID       *renderTargets,
    int32_t           renderTargetsNum,
    VAContextID       *context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::DestroyContext (
    VADriverContextP  ctx,
    VAContextID       context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::CreateBuffer (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferType      type,
    uint32_t          size,
    uint32_t          elementsNum,
    void              *data,
    VABufferID        *bufId)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       renderTarget)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::RenderPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           buffersNum)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::EndPicture (
    VADriverContextP  ctx,
    VAContextID       context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::QueryVideoProcPipelineCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID          *filters,
    uint32_t            filtersNum,
    VAProcPipelineCaps  *pipelineCaps)
{
    PDDI_MEDIA_CONTEXT mediaCtx = nullptr;

    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,          "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pipelineCaps, "nullptr pipelineCaps", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (filtersNum > 0)
    {
        DDI_CHK_NULL(filters,   "nullptr filters",     VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    mediaCtx   = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pipelineCaps->pipeline_flags             = VA_PROC_PIPELINE_FAST;
    pipelineCaps->filter_flags               = 0;
    pipelineCaps->rotation_flags             = (1 << VA_ROTATION_NONE) | (1 << VA_ROTATION_90) | (1 << VA_ROTATION_180) | (1 << VA_ROTATION_270);
    pipelineCaps->mirror_flags               = VA_MIRROR_HORIZONTAL  | VA_MIRROR_VERTICAL;
    pipelineCaps->blend_flags                = VA_BLEND_GLOBAL_ALPHA | VA_BLEND_PREMULTIPLIED_ALPHA | VA_BLEND_LUMA_KEY;
    pipelineCaps->num_forward_references     = DDI_CODEC_NUM_FWD_REF;
    pipelineCaps->num_backward_references    = DDI_CODEC_NUM_BK_REF;
    pipelineCaps->input_color_standards      = const_cast<VAProcColorStandardType*>(m_vpInputColorStd);
    pipelineCaps->num_input_color_standards  = DDI_VP_NUM_INPUT_COLOR_STD;
    pipelineCaps->output_color_standards     = const_cast<VAProcColorStandardType*>(m_vpOutputColorStd);
    pipelineCaps->num_output_color_standards = DDI_VP_NUM_OUT_COLOR_STD;

    pipelineCaps->num_input_pixel_formats    = 1;
    pipelineCaps->input_pixel_format[0]      = VA_FOURCC_NV12;
    pipelineCaps->num_output_pixel_formats   = 1;
    pipelineCaps->output_pixel_format[0]     = VA_FOURCC_NV12;

    if((MEDIA_IS_SKU(&(mediaCtx->SkuTable), FtrHCP2SFCPipe)))
    {
        pipelineCaps->max_input_width        = DDI_DECODE_HCP_SFC_MAX_WIDTH;
        pipelineCaps->max_input_height       = DDI_DECODE_HCP_SFC_MAX_HEIGHT;
    }
    else
    {
        pipelineCaps->max_input_width        = DDI_DECODE_SFC_MAX_WIDTH;
        pipelineCaps->max_input_height       = DDI_DECODE_SFC_MAX_HEIGHT;
    }
    pipelineCaps->min_input_width            = DDI_DECODE_SFC_MIN_WIDTH;
    pipelineCaps->min_input_height           = DDI_DECODE_SFC_MIN_HEIGHT;
    pipelineCaps->max_output_width           = DDI_DECODE_SFC_MAX_WIDTH;
    pipelineCaps->max_output_height          = DDI_DECODE_SFC_MAX_HEIGHT;
    pipelineCaps->min_output_width           = DDI_DECODE_SFC_MIN_WIDTH;
    pipelineCaps->min_output_height          = DDI_DECODE_SFC_MIN_HEIGHT;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::StatusCheck(
    PDDI_MEDIA_CONTEXT mediaCtx,
    DDI_MEDIA_SURFACE  *surface,
    VASurfaceID        surfaceId)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::QuerySurfaceError(
    VADriverContextP ctx,
    VASurfaceID      renderTarget,
    VAStatus         errorStatus,
    void             **errorInfo)
{
    return VA_STATUS_SUCCESS;
}