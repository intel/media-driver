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
//! \file     media_libva_util_next.cpp
//! \brief    libva util next implementaion.
//!
#include <sys/time.h>
#include "inttypes.h"
#include "media_libva_util_next.h"
#include "media_interfaces_mcpy_next.h"
#include "mos_utilities.h"
#include "mos_os.h"
#include "mos_defs.h"
#include "hwinfo_linux.h"
#include "ddi_decode_base_specific.h"
#include "media_ddi_encode_base.h"
//#include "ddi_libva_decoder_specific.h"
#include "media_libva_decoder.h"
#include "media_libva_encoder.h"
#include "memory_policy_manager.h"
#include "drm_fourcc.h"

// will remove when mtl open source
#define INTEL_PRELIM_ID_FLAG         (1ULL << 55)

#define intel_prelim_fourcc_mod_code(val) \
    (fourcc_mod_code(INTEL, (val)) | INTEL_PRELIM_ID_FLAG)

/* this definition is to avoid duplicate drm_fourcc.h this file is updated seldom */
#ifndef I915_FORMAT_MOD_4_TILED_MTL_MC_CCS
#define I915_FORMAT_MOD_4_TILED_MTL_MC_CCS    fourcc_mod_code(INTEL, 14)
#endif
#ifndef I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC
#define I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC    fourcc_mod_code(INTEL, 15)
#endif
#ifndef I915_FORMAT_MOD_4_TILED_LNL_CCS
#define I915_FORMAT_MOD_4_TILED_LNL_CCS    fourcc_mod_code(INTEL, 16)
#endif
#ifndef I915_FORMAT_MOD_4_TILED_BMG_CCS 
#define I915_FORMAT_MOD_4_TILED_BMG_CCS     fourcc_mod_code(INTEL, 17)
#endif

// default protected surface tag
#define PROTECTED_SURFACE_TAG   0x3000f

int32_t         MediaLibvaUtilNext::m_frameCountFps             = -1;
struct timeval  MediaLibvaUtilNext::m_tv1                       = {};
pthread_mutex_t MediaLibvaUtilNext::m_fpsMutex                  = PTHREAD_MUTEX_INITIALIZER;
int32_t         MediaLibvaUtilNext::m_vaFpsSampleSize           = 100;
bool            MediaLibvaUtilNext::m_isMediaFpsPrintFpsEnabled = false;

VAStatus MediaLibvaUtilNext::SetDefaultTileFormat(
    DDI_MEDIA_FORMAT             format,
    uint32_t                     surfaceUsageHint,
    MEDIA_FEATURE_TABLE          *skuTable,
    MEDIA_SURFACE_ALLOCATE_PARAM &params
    )
{
    VAStatus status        = VA_STATUS_SUCCESS;
    uint32_t tileFormat    = TILING_Y;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(skuTable, "skuTable is nullptr", VA_STATUS_ERROR_INVALID_PARAMETER);

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
        case Media_Format_R10G10B10X2:
        case Media_Format_B10G10R10X2:
        case Media_Format_A16R16G16B16:
        case Media_Format_A16B16G16R16:
        case Media_Format_YV12:
        case Media_Format_I420:
        case Media_Format_IYUV:
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != surfaceUsageHint   &&
                !(surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_VPP_WRITE))
            {
                tileFormat = TILING_NONE;
            }
            break;
        case Media_Format_RGBP:
        case Media_Format_BGRP:
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != surfaceUsageHint   &&
                !(surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_VPP_WRITE))
            {
                if(!(surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_DECODER))
                {
                    tileFormat = TILING_NONE;
                    break;
                }
                //Planar type surface align 32 to improve performance.
                params.alignedHeight = MOS_ALIGN_CEIL(params.height, 32);
            }
            params.alignedWidth = MOS_ALIGN_CEIL(params.width, 8);
            if (surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_VPP_WRITE)
            {
                if (format == Media_Format_RGBP)
                {
                    //Planar type surface align 32 to improve performance.
                    params.alignedHeight = MOS_ALIGN_CEIL(params.height, 32);
                }
            }
            tileFormat = TILING_Y;
            break;
        case Media_Format_A8R8G8B8:
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != surfaceUsageHint     &&
                !(surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_DECODER)   &&
                !(surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_VPP_WRITE) &&
                !(MEDIA_IS_SKU(skuTable, FtrRenderCompressionOnly)           &&
                  MEDIA_IS_SKU(skuTable, FtrE2ECompression)))
            {
                tileFormat = TILING_NONE;
            }
            break;
        case Media_Format_NV12:
        case Media_Format_NV21:
        case Media_Format_444P:
        case Media_Format_422H:
        case Media_Format_411P:
        case Media_Format_422V:
        case Media_Format_IMC3:
        case Media_Format_400P:
        case Media_Format_P010:
        case Media_Format_P012:
        case Media_Format_P016:
        case Media_Format_YUY2:
        case Media_Format_Y210:
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y212:
#endif
        case Media_Format_Y216:
        case Media_Format_AYUV:
#if VA_CHECK_VERSION(1, 13, 0)
        case Media_Format_XYUV:
#endif
        case Media_Format_Y410:
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y412:
#endif
        case Media_Format_Y416:
        case Media_Format_Y8:
        case Media_Format_Y16S:
        case Media_Format_Y16U:
        case Media_Format_VYUY:
        case Media_Format_YVYU:
        case Media_Format_UYVY:
            if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER != surfaceUsageHint &&
                !(surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_VPP_WRITE))
            {
                //Planar type surface align 32 to improve performance.
                params.alignedHeight = MOS_ALIGN_CEIL(params.height, 32);
            }
            params.alignedWidth = MOS_ALIGN_CEIL(params.width, 8);
            if (surfaceUsageHint & VA_SURFACE_ATTRIB_USAGE_HINT_VPP_WRITE)
            {
                if ((format == Media_Format_NV12) || (format == Media_Format_P010))
                {
                    //Planar type surface align 32 to improve performance.
                    params.alignedHeight = MOS_ALIGN_CEIL(params.height, 32);
                }
            }
            tileFormat = TILING_Y;
            break;
        case Media_Format_Buffer:
            tileFormat = TILING_NONE;
            break;
        default:
            tileFormat = TILING_NONE;
            DDI_ASSERTMESSAGE("Unsupported format");
            status = VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    if (VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER & surfaceUsageHint)
    {
        params.alignedWidth  = MOS_ALIGN_CEIL(params.alignedWidth, 16);
        params.alignedHeight = MOS_ALIGN_CEIL(params.alignedHeight, 16);
    }
    params.tileFormat    = tileFormat;
    return status;
}

void MediaLibvaUtilNext::InitSurfaceAllocateParams(
    MEDIA_SURFACE_ALLOCATE_PARAM &params,
    int32_t                      width,
    int32_t                      height,
    DDI_MEDIA_FORMAT             format,
    int                          memType,
    uint32_t                     surfaceUsageHint)
{
    DDI_FUNC_ENTER;
    params.pitch          = 0;
    params.tileFormat     = TILING_NONE;
    params.alignedWidth   = params.width = width;
    params.alignedHeight  = params.height = height;
    params.format         = format;
    params.cpTag          = 0;
    params.memType        = memType;
#ifdef _MMC_SUPPORTED
    params.bMemCompEnable = true;
#else
    params.bMemCompEnable = false;
#endif
    params.bMemCompRC     = false;
    return;
}

VAStatus MediaLibvaUtilNext::SetSurfaceParameterFromModifier(
    MEDIA_SURFACE_ALLOCATE_PARAM &params,
    uint64_t                     modifier)
{
    DDI_FUNC_ENTER;
    switch (modifier)
    {
        case I915_FORMAT_MOD_4_TILED:
            params.tileFormat = TILING_Y;
            params.bMemCompEnable = false;
            break;
        case I915_FORMAT_MOD_4_TILED_LNL_CCS:
        case I915_FORMAT_MOD_4_TILED_BMG_CCS:
            params.tileFormat = TILING_Y;
            params.bMemCompEnable = true;
            break;
        case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC:
            params.tileFormat = TILING_Y;
            params.bMemCompEnable = true;
            params.bMemCompRC = true;
            break;
        case I915_FORMAT_MOD_4_TILED_MTL_MC_CCS:
            params.tileFormat = TILING_Y;
            params.bMemCompEnable = true;
            params.bMemCompRC = false;
            break;
        case DRM_FORMAT_MOD_LINEAR:
            params.tileFormat = TILING_NONE;
            params.bMemCompEnable = false;
            break;
        case I915_FORMAT_MOD_X_TILED:
            params.tileFormat = TILING_X;
            params.bMemCompEnable = false;
            break;
        case I915_FORMAT_MOD_Y_TILED:
        case I915_FORMAT_MOD_Yf_TILED:
            params.tileFormat = TILING_Y;
            params.bMemCompEnable = false;
            break;
        case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS:
            params.tileFormat = TILING_Y;
            params.bMemCompEnable = true;
            params.bMemCompRC = true;
            break;
        case I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS:
            params.tileFormat = TILING_Y;
            params.bMemCompEnable = true;
            params.bMemCompRC = false;
            break;
        default:
            DDI_ASSERTMESSAGE("Unsupported modifier.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaUtilNext::GenerateGmmParamsForNoneCompressionExternalSurface(
    GMM_RESCREATE_CUSTOM_PARAMS  &gmmCustomParams,
    MEDIA_SURFACE_ALLOCATE_PARAM &params,
    PDDI_MEDIA_SURFACE           mediaSurface)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaSurface,            "mediaSurface is nullptr",      VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaSurface->pSurfDesc, "mediaSurface desc is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);

    int32_t baseHeight = 0;
    DDI_CHK_CONDITION(mediaSurface->pSurfDesc->uiPlanes == 0,
        "Invalid plane number.",
        VA_STATUS_ERROR_INVALID_PARAMETER);

    if (mediaSurface->pSurfDesc->uiPlanes == 1)
    {
        DDI_CHK_CONDITION(mediaSurface->pSurfDesc->uiSize == 0,
            "Invalid Size.",
            VA_STATUS_ERROR_INVALID_PARAMETER);
        baseHeight = mediaSurface->pSurfDesc->uiSize / params.pitch;
    }
    else
    {
        DDI_CHK_CONDITION(mediaSurface->pSurfDesc->uiOffsets[1] == 0,
            "Invalid offset.",
            VA_STATUS_ERROR_INVALID_PARAMETER);
        baseHeight = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
    }

    // Create GmmResourceInfo
    gmmCustomParams.Type          = RESOURCE_2D;
    gmmCustomParams.Format        = ConvertMediaFmtToGmmFmt(params.format);
    if ((params.format == Media_Format_YV12) || \
        (params.format == Media_Format_I420) || \
        (params.format == Media_Format_IYUV) || \
        (params.format == Media_Format_NV12) || \
        (params.format == Media_Format_P010) || \
        (params.format == Media_Format_P012) || \
        (params.format == Media_Format_P016) || \
        (params.format == Media_Format_NV21)) {
        // Align width to 2 for specific planar formats to handle
        // odd dimensions for external non-compressible surfaces
        gmmCustomParams.BaseWidth64 = MOS_ALIGN_CEIL(params.width, 2);
    } else {
        gmmCustomParams.BaseWidth64 = params.width;
    }
    gmmCustomParams.BaseHeight    = baseHeight;
    gmmCustomParams.Pitch         = params.pitch;
    gmmCustomParams.Size          = mediaSurface->pSurfDesc->uiSize;
    gmmCustomParams.BaseAlignment = 4096;
    gmmCustomParams.NoOfPlanes    = mediaSurface->pSurfDesc->uiPlanes;
    gmmCustomParams.CpTag         = params.cpTag;
    switch (params.tileFormat)
    {
        case TILING_Y:
            gmmCustomParams.Flags.Info.TiledY = true;
            break;
        case TILING_X:
            gmmCustomParams.Flags.Info.TiledX = true;
            break;
        case TILING_NONE:
        default:
            gmmCustomParams.Flags.Info.Linear = true;
    }


    // Init NotCompressed flag as true to default Create as uncompressed surface on Xe2 Compression.
    gmmCustomParams.Flags.Info.NotCompressed = 1;

    switch (mediaSurface->pSurfDesc->uiPlanes)
    {
        case 1:
            gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
            gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
            break;
        case 2:
            gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
            gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
            gmmCustomParams.PlaneOffset.X[GMM_PLANE_U] = 0;
            gmmCustomParams.PlaneOffset.Y[GMM_PLANE_U] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
            gmmCustomParams.PlaneOffset.X[GMM_PLANE_V] = 0;
            gmmCustomParams.PlaneOffset.Y[GMM_PLANE_V] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
            break;
        case 3:
            if (mediaSurface->format == Media_Format_YV12)
            {
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_U] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_U] = mediaSurface->pSurfDesc->uiOffsets[2] / params.pitch;
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_V] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_V] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
            }
            else
            {
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_U] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_U] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_V] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_V] = mediaSurface->pSurfDesc->uiOffsets[2] / params.pitch;
            }
            break;
        default:
            DDI_ASSERTMESSAGE("Invalid plane number.");
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaUtilNext::GenerateGmmParamsForCompressionExternalSurface(
    GMM_RESCREATE_CUSTOM_PARAMS_2 &gmmCustomParams,
    MEDIA_SURFACE_ALLOCATE_PARAM &params,
    PDDI_MEDIA_SURFACE           mediaSurface,
    PDDI_MEDIA_CONTEXT           mediaDrvCtx)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaSurface,            "mediaSurface is nullptr",      VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaSurface->pSurfDesc, "mediaSurface desc is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaDrvCtx, "media context is nullptr", VA_STATUS_ERROR_INVALID_CONTEXT);
    // Create GmmResourceInfo
    gmmCustomParams.Type          = RESOURCE_2D;
    gmmCustomParams.Format        = ConvertMediaFmtToGmmFmt(params.format);
    if ((params.format == Media_Format_YV12) || \
        (params.format == Media_Format_I420) || \
        (params.format == Media_Format_IYUV) || \
        (params.format == Media_Format_NV12) || \
        (params.format == Media_Format_NV21)) {
        // Align width to 2 for specific planar formats to handle
        // odd dimensions for external non-compressible surfaces
        gmmCustomParams.BaseWidth64 = MOS_ALIGN_CEIL(params.alignedWidth, 2);
    } else {
        gmmCustomParams.BaseWidth64 = params.alignedWidth;
    }
    gmmCustomParams.BaseHeight    = params.alignedHeight;
    gmmCustomParams.Pitch         = params.pitch;
    gmmCustomParams.Size          = mediaSurface->pSurfDesc->uiSize;
    gmmCustomParams.BaseAlignment = 4096;
    gmmCustomParams.NoOfPlanes    = mediaSurface->pSurfDesc->uiPlanes;
    gmmCustomParams.CpTag         = params.cpTag;

    if (MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrXe2Compression))
    {
        // Init NotCompressed flag as false to default Create as compressed surface w/ Xe2 Compression.
        gmmCustomParams.Flags.Info.NotCompressed = 0;
    }

    switch (params.tileFormat)
    {
        case TILING_Y:
            gmmCustomParams.Flags.Info.TiledY = true;
            gmmCustomParams.Flags.Gpu.MMC    = false;
            if (MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrE2ECompression) &&
                (!MEDIA_IS_WA(&mediaDrvCtx->WaTable, WaDisableVPMmc)    &&
                !MEDIA_IS_WA(&mediaDrvCtx->WaTable, WaDisableCodecMmc)) &&
                params.bMemCompEnable)
            {
                gmmCustomParams.Flags.Gpu.MMC               = true;
                gmmCustomParams.Flags.Info.MediaCompressed  = 1;
                gmmCustomParams.Flags.Info.RenderCompressed = 0;
                gmmCustomParams.Flags.Gpu.CCS               = 1;
                gmmCustomParams.Flags.Gpu.RenderTarget      = 1;
                gmmCustomParams.Flags.Gpu.UnifiedAuxSurface = 1;

                if (params.bMemCompRC)
                {
                    gmmCustomParams.Flags.Info.MediaCompressed  = 0;
                    gmmCustomParams.Flags.Info.RenderCompressed = 1;
                }

                if(MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrRenderCompressionOnly))
                {
                    gmmCustomParams.Flags.Info.MediaCompressed = 0;

                    if (params.format == Media_Format_X8R8G8B8 ||
                        params.format == Media_Format_X8B8G8R8 ||
                        params.format == Media_Format_A8B8G8R8 ||
                        params.format == Media_Format_A8R8G8B8 ||
                        params.format == Media_Format_R8G8B8A8)
                    {
                        gmmCustomParams.Flags.Info.MediaCompressed  = 0;
                        gmmCustomParams.Flags.Info.RenderCompressed = 1;
                    }
                }

                if(MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrFlatPhysCCS))
                {
                    gmmCustomParams.Flags.Gpu.UnifiedAuxSurface = 0;
                }
            }
            break;
        case TILING_X:
            gmmCustomParams.Flags.Info.TiledX = true;
            break;
        case TILING_NONE:
        default:
            gmmCustomParams.Flags.Info.Linear = true;
    }

    if(MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrFlatPhysCCS))
    {
        switch (mediaSurface->pSurfDesc->uiPlanes)
        {
            case 1:
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                break;
            case 2:
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_U] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_U] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_V] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_V] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
                break;
            case 3:
                if (mediaSurface->format == Media_Format_YV12)
                {
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_U] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_U] = mediaSurface->pSurfDesc->uiOffsets[2] / params.pitch;
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_V] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_V] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
                }
                else
                {
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_U] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_U] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_V] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_V] = mediaSurface->pSurfDesc->uiOffsets[2] / params.pitch;
                }
                break;
            default:
                DDI_ASSERTMESSAGE("Invalid plane number.");
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }
    else
    {
        // build Aux SW map for platfroms w/o physical map
        gmmCustomParams.AuxSurf.BaseAlignment = {0};
        gmmCustomParams.NoOfPlanes = mediaSurface->pSurfDesc->uiPlanes/2;
        gmmCustomParams.Size = (gmmCustomParams.NoOfPlanes == 1) ? mediaSurface->pSurfDesc->uiOffsets[1]:mediaSurface->pSurfDesc->uiOffsets[2];
        switch(gmmCustomParams.NoOfPlanes)
        {
            case 1:
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                gmmCustomParams.AuxSurf.Size = mediaSurface->pSurfDesc->uiSize - gmmCustomParams.Size;
                gmmCustomParams.AuxSurf.Pitch = mediaSurface->pSurfDesc->uiPitches[1];
                gmmCustomParams.AuxSurf.PlaneOffset.X[GMM_PLANE_Y] = 0;
                gmmCustomParams.AuxSurf.PlaneOffset.Y[GMM_PLANE_Y] = 0;
                break;
            case 2:
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_U] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_U] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
                gmmCustomParams.PlaneOffset.X[GMM_PLANE_V] = 0;
                gmmCustomParams.PlaneOffset.Y[GMM_PLANE_V] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
                gmmCustomParams.AuxSurf.Size = (mediaSurface->pSurfDesc->uiOffsets[3] - mediaSurface->pSurfDesc->uiOffsets[2]) * 2;
                gmmCustomParams.AuxSurf.Pitch = mediaSurface->pSurfDesc->uiPitches[2];
                gmmCustomParams.AuxSurf.PlaneOffset.X[GMM_PLANE_Y] = 0;
                gmmCustomParams.AuxSurf.PlaneOffset.Y[GMM_PLANE_Y] = 0;
                gmmCustomParams.AuxSurf.PlaneOffset.X[GMM_PLANE_U] = (mediaSurface->pSurfDesc->uiOffsets[3]
                                                            - mediaSurface->pSurfDesc->uiOffsets[2]);
                gmmCustomParams.AuxSurf.PlaneOffset.Y[GMM_PLANE_U] = 0;
                gmmCustomParams.AuxSurf.PlaneOffset.X[GMM_PLANE_V] = (mediaSurface->pSurfDesc->uiOffsets[3]
                                                            - mediaSurface->pSurfDesc->uiOffsets[2]);
                gmmCustomParams.AuxSurf.PlaneOffset.Y[GMM_PLANE_V] = 0;
                break;
            case 3:
                if (mediaSurface->format == Media_Format_YV12)
                {
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_U] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_U] = mediaSurface->pSurfDesc->uiOffsets[2] / params.pitch;
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_V] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_V] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
                }
                else
                {
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_Y] = mediaSurface->pSurfDesc->uiOffsets[0] / params.pitch;
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_U] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_U] = mediaSurface->pSurfDesc->uiOffsets[1] / params.pitch;
                    gmmCustomParams.PlaneOffset.X[GMM_PLANE_V] = 0;
                    gmmCustomParams.PlaneOffset.Y[GMM_PLANE_V] = mediaSurface->pSurfDesc->uiOffsets[2] / params.pitch;
                }
                break;
            default:
                DDI_ASSERTMESSAGE("Invalid plane number.");
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaUtilNext::CreateExternalSurface(
    MEDIA_SURFACE_ALLOCATE_PARAM &params,
    PDDI_MEDIA_SURFACE           mediaSurface,
    PDDI_MEDIA_CONTEXT           mediaDrvCtx)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaSurface,                   "mediaSurface is nullptr",      VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaSurface->pSurfDesc,        "mediaSurface desc is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaDrvCtx,                    "media context is nullptr",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaDrvCtx->pDrmBufMgr,        "drm buffer mgr is nullptr",    VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaDrvCtx->pGmmClientContext, "gmm context is nullptr",       VA_STATUS_ERROR_INVALID_CONTEXT);

    GMM_RESOURCE_INFO  *gmmResourceInfo = nullptr;
    MOS_LINUX_BO       *bo = nullptr;
    uint32_t           swizzle_mode;
    unsigned int       patIndex = PAT_INDEX_INVALID;
    VAStatus           status = VA_STATUS_SUCCESS;

    // Default set as compression not supported, surface compression import only support from Memory Type VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2 or VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3
    params.bMemCompEnable = false;
    params.bMemCompRC = false;
    params.pitch = mediaSurface->pSurfDesc->uiPitches[0];
    DDI_CHK_CONDITION(params.pitch == 0, "Invalid pich.", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Set cp flag to indicate the secure surface
    if (mediaSurface->pSurfDesc->uiFlags & VA_SURFACE_EXTBUF_DESC_PROTECTED)
    {
        params.cpTag = PROTECTED_SURFACE_TAG;
    }

    switch(mediaSurface->pSurfDesc->uiVaMemType)
    {
        case VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM:
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME:
            params.tileFormat = mediaSurface->pSurfDesc->uiFlags & VA_SURFACE_EXTBUF_DESC_ENABLE_TILING ? TILING_Y : TILING_NONE;
            break;
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2:
            params.pitch = mediaSurface->pSurfDesc->uiPitches[0];
            status = SetSurfaceParameterFromModifier(params, mediaSurface->pSurfDesc->modifier);
            if(status != VA_STATUS_SUCCESS)
            {
                DDI_ASSERTMESSAGE("Set surface from modifier failed.");
                return status;
            }
            break;
#if VA_CHECK_VERSION(1, 21, 0)
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3:
            params.pitch = mediaSurface->pSurfDesc->uiPitches[0];
            status = SetSurfaceParameterFromModifier(params, mediaSurface->pSurfDesc->modifier);
            if(status != VA_STATUS_SUCCESS)
            {
                DDI_ASSERTMESSAGE("Set surface from modifier failed.");
                return status;
            }
            break;
#endif
        case VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR:
            params.tileFormat = TILE_NONE;
            break;
    }


    GMM_RESCREATE_CUSTOM_PARAMS_2 gmmCustomParams;
    MOS_ZeroMemory(&gmmCustomParams, sizeof(gmmCustomParams));
    if (VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR == mediaSurface->pSurfDesc->uiVaMemType)
    {
        gmmCustomParams.Usage = GMM_RESOURCE_USAGE_STAGING;
        gmmCustomParams.Type = RESOURCE_1D;
    }

    if (params.bMemCompEnable)
    {
        status = GenerateGmmParamsForCompressionExternalSurface(gmmCustomParams, params, mediaSurface, mediaDrvCtx);
        if(status != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("Generate gmmParams for compression external surface failed.");
            return status;
        }

        gmmResourceInfo = mediaDrvCtx->pGmmClientContext->CreateCustomResInfoObject_2(&gmmCustomParams);
    }
    else
    {
        status = GenerateGmmParamsForNoneCompressionExternalSurface(gmmCustomParams, params, mediaSurface);
        if(status != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("Generate gmmParams for none compression external surface failed.");
            return status;
        }

        gmmResourceInfo = mediaDrvCtx->pGmmClientContext->CreateCustomResInfoObject(&gmmCustomParams);
    }

    DDI_CHK_NULL(gmmResourceInfo, "Gmm create resource failed", VA_STATUS_ERROR_ALLOCATION_FAILED);

    patIndex = MosInterface::GetPATIndexFromGmm(mediaDrvCtx->pGmmClientContext, gmmResourceInfo);

    // DRM buffer allocated by Application, No need to re-allocate new DRM buffer
    switch (mediaSurface->pSurfDesc->uiVaMemType)
    {
        case VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM:
            bo = mos_bo_create_from_name(mediaDrvCtx->pDrmBufMgr, "MEDIA", mediaSurface->pSurfDesc->ulBuffer);
            break;
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME:
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2:
#if VA_CHECK_VERSION(1, 21, 0)
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3:
#endif
        {
            struct mos_drm_bo_alloc_prime alloc_prime;
            alloc_prime.name = "prime";
            alloc_prime.prime_fd = mediaSurface->pSurfDesc->ulBuffer;
            alloc_prime.size = mediaSurface->pSurfDesc->uiSize;
            alloc_prime.pat_index = patIndex;

            bo = mos_bo_create_from_prime(mediaDrvCtx->pDrmBufMgr, &alloc_prime);
        }
            break;
        case VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR:
        {
            struct mos_drm_bo_alloc_userptr alloc_uptr;
            alloc_uptr.name = "SysSurface";
            alloc_uptr.addr = (void *)mediaSurface->pSurfDesc->ulBuffer;
            alloc_uptr.tiling_mode = mediaSurface->pSurfDesc->uiTile;
            alloc_uptr.stride = params.pitch;
            alloc_uptr.size = mediaSurface->pSurfDesc->uiBuffserSize;
            alloc_uptr.pat_index = patIndex;

            bo = mos_bo_alloc_userptr(mediaDrvCtx->pDrmBufMgr, &alloc_uptr);
        }
            break;
    }

    if (nullptr == bo)
    {
        DDI_ASSERTMESSAGE("Failed to create drm buffer object according to input buffer descriptor.");
        mediaDrvCtx->pGmmClientContext->DestroyResInfoObject(gmmResourceInfo);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    mediaSurface->pGmmResourceInfo = gmmResourceInfo;
    mediaSurface->bMapped          = false;
    mediaSurface->format           = params.format;
    mediaSurface->iWidth           = params.width;
    mediaSurface->iHeight          = gmmResourceInfo->GetBaseHeight();
    mediaSurface->iRealHeight      = params.height;
    mediaSurface->iPitch           = params.pitch;
    mediaSurface->iRefCount        = 0;
    mediaSurface->bo               = bo;
    mediaSurface->TileType         = params.tileFormat;
    mediaSurface->isTiled          = (params.tileFormat != TILING_NONE) ? 1 : 0;
    mediaSurface->pData            = (uint8_t*) bo->virt;
    DDI_VERBOSEMESSAGE("Allocate external surface %7d bytes (%d x %d resource).", mediaSurface->pSurfDesc->uiSize, params.width, params.height);
    uint32_t event[] = {bo->handle, params.format, params.width, params.height, params.pitch, bo->size, params.tileFormat, params.cpTag};
    MOS_TraceEventExt(EVENT_VA_SURFACE, EVENT_TYPE_INFO, event, sizeof(event), &gmmResourceInfo->GetResFlags(), sizeof(GMM_RESOURCE_FLAG));

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaUtilNext::GenerateGmmParamsForInternalSurface(
    GMM_RESCREATE_PARAMS         &gmmParams,
    MEDIA_SURFACE_ALLOCATE_PARAM &params,
    PDDI_MEDIA_CONTEXT           mediaDrvCtx)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaDrvCtx, "media context is nullptr", VA_STATUS_ERROR_INVALID_CONTEXT);

    MosUtilities::MosZeroMemory(&gmmParams, sizeof(gmmParams));
    gmmParams.BaseWidth         = params.alignedWidth;
    gmmParams.BaseHeight        = params.alignedHeight;
    gmmParams.ArraySize         = 1;
    gmmParams.Type              = RESOURCE_2D;
    gmmParams.Format            = ConvertMediaFmtToGmmFmt(params.format);
    
    DDI_CHK_CONDITION(gmmParams.Format == GMM_FORMAT_INVALID, "Unsupported format", VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT);

    if (MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrXe2Compression))
    {
        // Init NotCompressed flag as true to default Create as uncompressed surface on Xe2 Compression.
        gmmParams.Flags.Info.NotCompressed = 1;
    }

    switch (params.tileFormat)
    {
        case TILING_Y:
            // Disable MMC for application required surfaces, because some cases' output streams have corruption.
            gmmParams.Flags.Gpu.MMC    = false;
            if (MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrE2ECompression)             &&
                (!MEDIA_IS_WA(&mediaDrvCtx->WaTable, WaDisableVPMmc)                &&
                !MEDIA_IS_WA(&mediaDrvCtx->WaTable,  WaDisableCodecMmc))            &&
                MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrCompressibleSurfaceDefault) &&
                params.bMemCompEnable)
            {
                gmmParams.Flags.Gpu.MMC               = true;
                gmmParams.Flags.Info.MediaCompressed  = 1;
                gmmParams.Flags.Info.RenderCompressed = 0;
                gmmParams.Flags.Gpu.CCS               = 1;
                gmmParams.Flags.Gpu.RenderTarget      = 1;
                gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;

                if (params.bMemCompRC)
                {
                    gmmParams.Flags.Info.MediaCompressed  = 0;
                    gmmParams.Flags.Info.RenderCompressed = 1;
                }

                if(MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrXe2Compression))
                {
                    gmmParams.Flags.Info.NotCompressed = 0;
                }

                if (MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrRenderCompressionOnly))
                {
                    gmmParams.Flags.Info.MediaCompressed = 0;

                    if (params.format == Media_Format_X8R8G8B8 ||
                        params.format == Media_Format_X8B8G8R8 ||
                        params.format == Media_Format_A8B8G8R8 ||
                        params.format == Media_Format_A8R8G8B8 ||
                        params.format == Media_Format_R8G8B8A8)
                    {
                        gmmParams.Flags.Info.MediaCompressed  = 0;
                        gmmParams.Flags.Info.RenderCompressed = 1;
                    }
                    else
                    {
                        gmmParams.Flags.Gpu.MMC               = false;
                        gmmParams.Flags.Info.MediaCompressed  = 0;
                        gmmParams.Flags.Info.RenderCompressed = 0;
                        gmmParams.Flags.Gpu.CCS               = 0;
                        gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;
                    }
                }
            }
            // For ARGB surface, always allocate it as tile4.
            // This is a WA for ExportSurfaceHandle because modifer for tile64 isn't defined.
            if ((params.format == Media_Format_A8R8G8B8 || 
                params.format == Media_Format_B10G10R10A2 || 
                params.format == Media_Format_A8B8G8R8 ||
                params.format == Media_Format_X8R8G8B8) && !MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrTileY))
            {
                gmmParams.Flags.Info.Tile4 = true;
            }
            break;
        case TILING_X:
            gmmParams.Flags.Info.TiledX    = true;
            break;
        default:
            gmmParams.Flags.Info.Linear    = true;
    }
    gmmParams.Flags.Gpu.Video = true;
    gmmParams.Flags.Info.LocalOnly = MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrLocalMemory);

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaUtilNext::CreateInternalSurface(
    MEDIA_SURFACE_ALLOCATE_PARAM &params,
    PDDI_MEDIA_SURFACE           mediaSurface,
    PDDI_MEDIA_CONTEXT           mediaDrvCtx)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaSurface,                   "mediaSurface is nullptr",      VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaDrvCtx,                    "media context is nullptr",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaDrvCtx->pGmmClientContext, "gmm context is nullptr",       VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus              status           = VA_STATUS_SUCCESS;
    MOS_LINUX_BO          *bo              = nullptr;
    GMM_RESCREATE_PARAMS  gmmParams        = {};
    GMM_RESOURCE_INFO     *gmmResourceInfo = nullptr;
    if (mediaSurface->pSurfDesc)
    {
        if (mediaSurface->pSurfDesc->uiFlags & VA_SURFACE_EXTBUF_DESC_ENABLE_TILING )
        {
            params.tileFormat = TILING_Y;
        }
        else if (mediaSurface->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_VA)
        {
            params.tileFormat = TILING_NONE;
            params.alignedHeight = params.height;
        }
    }

    status = GenerateGmmParamsForInternalSurface(gmmParams, params, mediaDrvCtx);
    if(status != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Generate gmmParams for internal surface failed.");
        return status;
    }
    
    mediaSurface->pGmmResourceInfo = gmmResourceInfo = mediaDrvCtx->pGmmClientContext->CreateResInfoObject(&gmmParams);
    DDI_CHK_NULL(gmmResourceInfo, "Gmm create resource failed", VA_STATUS_ERROR_ALLOCATION_FAILED);

    uint32_t  gmmPitch  = (uint32_t)gmmResourceInfo->GetRenderPitch();
    uint32_t  gmmSize   = (uint32_t)gmmResourceInfo->GetSizeSurface();
    uint32_t  gmmHeight = gmmResourceInfo->GetBaseHeight();

    if (0 == gmmPitch || 0 == gmmSize || 0 == gmmHeight)
    {
        DDI_ASSERTMESSAGE("Gmm Create Resource Failed.");
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    switch (gmmResourceInfo->GetTileType())
    {
        case GMM_TILED_Y:
            params.tileFormat = TILING_Y;
            break;
        case GMM_TILED_X:
            params.tileFormat = TILING_X;
            break;
        case GMM_NOT_TILED:
            params.tileFormat = TILING_NONE;
            break;
        default:
            params.tileFormat = TILING_Y;
            break;
    }

    MemoryPolicyParameter memPolicyPar;
    MosUtilities::MosZeroMemory(&memPolicyPar, sizeof(MemoryPolicyParameter));

    memPolicyPar.skuTable = &mediaDrvCtx->SkuTable;
    memPolicyPar.waTable  = &mediaDrvCtx->WaTable;
    memPolicyPar.resInfo  = mediaSurface->pGmmResourceInfo;
    memPolicyPar.resName  = "Media Surface";
    memPolicyPar.preferredMemType = (MEDIA_IS_WA(&mediaDrvCtx->WaTable, WaForceAllocateLML4)) ? MOS_MEMPOOL_DEVICEMEMORY : params.memType;

    params.memType = MemoryPolicyManager::UpdateMemoryPolicy(&memPolicyPar);

    unsigned int patIndex = MosInterface::GetPATIndexFromGmm(mediaDrvCtx->pGmmClientContext, gmmResourceInfo);
    bool isCpuCacheable   = gmmResourceInfo->GetResFlags().Info.Cacheable;

    //This is wa for tile4 + compressed + scanout surface(!isCpuCacheable)
    bool is64kPageAlignmentNeed =
        MEDIA_IS_WA(&mediaDrvCtx->WaTable, WaTile4CompressScanoutSurf64KNeed)
        && gmmResourceInfo->GetResFlags().Info.Tile4
        && (gmmResourceInfo->GetResFlags().Info.MediaCompressed || gmmResourceInfo->GetResFlags().Info.RenderCompressed)
        && !isCpuCacheable;

    if ( params.tileFormat == TILING_NONE )
    {
        struct mos_drm_bo_alloc alloc;
        alloc.name = "Media";
        alloc.size = gmmSize;
        alloc.alignment = is64kPageAlignmentNeed ? PAGE_SIZE_64K : PAGE_SIZE_4K;
        alloc.ext.tiling_mode = TILING_NONE;
        alloc.ext.mem_type = params.memType;
        alloc.ext.pat_index = patIndex;
        alloc.ext.cpu_cacheable = isCpuCacheable;
        alloc.ext.scanout_surf  = !isCpuCacheable;
        bo = mos_bo_alloc(mediaDrvCtx->pDrmBufMgr, &alloc);
        params.pitch = gmmPitch;
    }
    else
    {
        struct mos_drm_bo_alloc_tiled alloc_tiled;
        alloc_tiled.name = "MEDIA";
        alloc_tiled.x = gmmPitch;
        alloc_tiled.y = (gmmSize + gmmPitch -1)/gmmPitch;
        alloc_tiled.cpp = 1;
        alloc_tiled.alignment = is64kPageAlignmentNeed ? PAGE_SIZE_64K : PAGE_SIZE_4K;
        alloc_tiled.ext.tiling_mode = params.tileFormat;
        alloc_tiled.ext.mem_type = params.memType;;
        alloc_tiled.ext.pat_index = patIndex;
        alloc_tiled.ext.cpu_cacheable = isCpuCacheable;
        alloc_tiled.ext.scanout_surf  = !isCpuCacheable;

        bo = mos_bo_alloc_tiled(mediaDrvCtx->pDrmBufMgr, &alloc_tiled);
        params.pitch = alloc_tiled.pitch;
    }

    mediaSurface->bMapped = false;
    DDI_CHK_NULL(bo, "Failed to create drm buffer object according to input buffer descriptor." ,VA_STATUS_ERROR_ALLOCATION_FAILED);

    mediaSurface->format      = params.format;
    mediaSurface->iWidth      = params.width;
    mediaSurface->iHeight     = gmmHeight;
    mediaSurface->iRealHeight = params.height;
    mediaSurface->iPitch      = params.pitch;
    mediaSurface->iRefCount   = 0;
    mediaSurface->bo          = bo;
    mediaSurface->TileType    = params.tileFormat;
    mediaSurface->isTiled     = (params.tileFormat != TILING_NONE) ? 1 : 0;
    mediaSurface->pData       = (uint8_t*) bo->virt;
    DDI_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource, gmmTiledType %d)).",gmmSize, params.width, params.height, gmmResourceInfo->GetTileType());
    uint32_t event[] = {bo->handle, params.format, params.width, params.height, params.pitch, bo->size, params.tileFormat, params.cpTag};
    MOS_TraceEventExt(EVENT_VA_SURFACE, EVENT_TYPE_INFO, event, sizeof(event), &gmmResourceInfo->GetResFlags(), sizeof(GMM_RESOURCE_FLAG));
    
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaUtilNext::AllocateSurface(
    DDI_MEDIA_FORMAT            format,
    int32_t                     width,
    int32_t                     height,
    PDDI_MEDIA_SURFACE          mediaSurface,
    PDDI_MEDIA_CONTEXT          mediaDrvCtx)
{
    VAStatus                     status = VA_STATUS_SUCCESS;
    MEDIA_SURFACE_ALLOCATE_PARAM params = {};
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaSurface,                   "mediaSurface is nullptr",                   VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaDrvCtx,                    "mediaDrvCtx is nullptr",                    VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaDrvCtx->pGmmClientContext, "mediaDrvCtx->pGmmClientContext is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);

    InitSurfaceAllocateParams(params, width, height, format,
        mediaSurface->memType, mediaSurface->surfaceUsageHint);

    status = SetDefaultTileFormat(format, mediaSurface->surfaceUsageHint, &mediaDrvCtx->SkuTable, params);
    if (status != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Get tile format failed.");
        return status;
    }

    if (IsExternalSurface(mediaSurface))
    {
        return CreateExternalSurface(params, mediaSurface, mediaDrvCtx);
    }
    else
    {
        return CreateInternalSurface(params, mediaSurface, mediaDrvCtx);
    }
}

bool MediaLibvaUtilNext::IsExternalSurface(PDDI_MEDIA_SURFACE surface)
{
    DDI_FUNC_ENTER;
    if (nullptr == surface)
    {
        return false;
    }
    else if (surface->pSurfDesc == nullptr)
    {
        return false;
    }
    else
    {
        if (surface->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM ||
            surface->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME ||
            surface->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2 ||
#if VA_CHECK_VERSION(1, 21, 0)
            surface->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 ||
#endif
            surface->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

PDDI_MEDIA_SURFACE_HEAP_ELEMENT MediaLibvaUtilNext::AllocPMediaSurfaceFromHeap(PDDI_MEDIA_HEAP surfaceHeap)
{
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  mediaSurfaceHeapElmt = nullptr;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surfaceHeap, "nullptr surfaceHeap", nullptr);

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

void MediaLibvaUtilNext::ReleasePMediaSurfaceFromHeap(PDDI_MEDIA_HEAP surfaceHeap, uint32_t vaSurfaceID)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surfaceHeap, "nullptr surfaceHeap", );

    DDI_CHK_LESS(vaSurfaceID, surfaceHeap->uiAllocatedHeapElements, "invalid surface id", );
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapBase                   = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surfaceHeap->pHeapBase;
    DDI_CHK_NULL(mediaSurfaceHeapBase, "nullptr mediaSurfaceHeapBase", );

    PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapElmt                   = &mediaSurfaceHeapBase[vaSurfaceID];
    DDI_CHK_NULL(mediaSurfaceHeapElmt->pSurface, "surface is already released", );
    void *firstFree                        = surfaceHeap->pFirstFreeHeapElement;
    surfaceHeap->pFirstFreeHeapElement     = (void*)mediaSurfaceHeapElmt;
    mediaSurfaceHeapElmt->pNextFree        = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)firstFree;
    mediaSurfaceHeapElmt->pSurface         = nullptr;
}

VAStatus MediaLibvaUtilNext::CreateSurface(DDI_MEDIA_SURFACE  *surface, PDDI_MEDIA_CONTEXT mediaDrvCtx)
{
    VAStatus hr = VA_STATUS_SUCCESS;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_BUFFER);

    // better to differentiate 1D and 2D type
    hr = AllocateSurface(surface->format,
                         surface->iWidth,
                         surface->iHeight,
                         surface,
                         mediaDrvCtx);
    if (VA_STATUS_SUCCESS == hr && nullptr != surface->bo)
    {
        surface->base = surface->name;
    }

    return hr;
}

GMM_RESOURCE_FORMAT MediaLibvaUtilNext::ConvertMediaFmtToGmmFmt(DDI_MEDIA_FORMAT format)
{
    DDI_FUNC_ENTER;
    switch (format)
    {
        case Media_Format_X8R8G8B8   : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case Media_Format_A8R8G8B8   : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case Media_Format_X8B8G8R8   : return GMM_FORMAT_R8G8B8X8_UNORM_TYPE;
        case Media_Format_A8B8G8R8   : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case Media_Format_R8G8B8A8   : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case Media_Format_R5G6B5     : return GMM_FORMAT_B5G6R5_UNORM_TYPE;
        case Media_Format_R8G8B8     : return GMM_FORMAT_R8G8B8_UNORM;
        case Media_Format_RGBP       : return GMM_FORMAT_RGBP;
        case Media_Format_BGRP       : return GMM_FORMAT_BGRP;
        case Media_Format_NV12       : return GMM_FORMAT_NV12_TYPE;
        case Media_Format_NV21       : return GMM_FORMAT_NV21_TYPE;
        case Media_Format_YUY2       : return GMM_FORMAT_YUY2;
        case Media_Format_YVYU       : return GMM_FORMAT_YVYU;
        case Media_Format_UYVY       : return GMM_FORMAT_UYVY;
        case Media_Format_VYUY       : return GMM_FORMAT_VYUY;
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
        case Media_Format_R10G10B10X2: return GMM_FORMAT_R10G10B10A2_UNORM_TYPE;
        case Media_Format_B10G10R10X2: return GMM_FORMAT_B10G10R10A2_UNORM_TYPE;
        case Media_Format_P012       : return GMM_FORMAT_P016_TYPE;
        case Media_Format_P016       : return GMM_FORMAT_P016_TYPE;
        case Media_Format_Y210       : return GMM_FORMAT_Y210_TYPE;
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y212       : return GMM_FORMAT_Y212_TYPE;
#endif
        case Media_Format_Y216       : return GMM_FORMAT_Y216_TYPE;
        case Media_Format_AYUV       : return GMM_FORMAT_AYUV_TYPE;
#if VA_CHECK_VERSION(1, 13, 0)
        case Media_Format_XYUV       : return GMM_FORMAT_AYUV_TYPE;
#endif
        case Media_Format_Y410       : return GMM_FORMAT_Y410_TYPE;
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y412       : return GMM_FORMAT_Y412_TYPE;
#endif
        case Media_Format_Y416       : return GMM_FORMAT_Y416_TYPE;
        case Media_Format_Y8         : return GMM_FORMAT_MEDIA_Y8_UNORM;
        case Media_Format_Y16S       : return GMM_FORMAT_MEDIA_Y16_SNORM;
        case Media_Format_Y16U       : return GMM_FORMAT_MEDIA_Y16_UNORM;
        case Media_Format_A16R16G16B16: return GMM_FORMAT_B16G16R16A16_UNORM;
        case Media_Format_A16B16G16R16: return GMM_FORMAT_R16G16B16A16_UNORM;
        default                      : return GMM_FORMAT_INVALID;
    }
}

void MediaLibvaUtilNext::WaitSemaphore(PMEDIA_SEM_T sem)
{
    DDI_FUNC_ENTER;
    int32_t ret = sem_wait(sem);
    if (ret != 0)
    {
        DDI_NORMALMESSAGE("wait semaphore error!\n");
    }
}

int32_t MediaLibvaUtilNext::TryWaitSemaphore(PMEDIA_SEM_T sem)
{
    return sem_trywait(sem);
}

void MediaLibvaUtilNext::PostSemaphore(PMEDIA_SEM_T sem)
{
    int32_t ret = sem_post(sem);
    if (ret != 0)
    {
        DDI_NORMALMESSAGE("post semaphore error!\n");
    }
}

void MediaLibvaUtilNext::DestroySemaphore(PMEDIA_SEM_T sem)
{
    int32_t ret = sem_destroy(sem);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't destroy the semaphore!\n");
    }
}

VAStatus MediaLibvaUtilNext::UnRegisterRTSurfaces(
    VADriverContextP    ctx,
    PDDI_MEDIA_SURFACE  surface)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx, "nullptr context!", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx   = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx!", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(surface,  "nullptr surface!",  VA_STATUS_ERROR_INVALID_PARAMETER);

    //Look through all decode contexts to unregister the surface in each decode context's RTtable.
    if (mediaCtx->pDecoderCtxHeap != nullptr)
    {
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT decVACtxHeapBase = nullptr;

        MosUtilities::MosLockMutex(&mediaCtx->DecoderMutex);
        decVACtxHeapBase  = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)mediaCtx->pDecoderCtxHeap->pHeapBase;
        for (uint32_t j = 0; j < mediaCtx->pDecoderCtxHeap->uiAllocatedHeapElements; j++)
        {
            if (decVACtxHeapBase[j].pVaContext != nullptr)
            {
                decode::PDDI_DECODE_CONTEXT decCtx = (decode::PDDI_DECODE_CONTEXT)decVACtxHeapBase[j].pVaContext;
                if (decCtx && decCtx->m_ddiDecodeNext)
                {
                    //not check the return value since the surface may not be registered in the context. pay attention to LOGW.
                    decCtx->m_ddiDecodeNext->UnRegisterRTSurfaces(&decCtx->RTtbl, surface);
                }
            }
        }
        MosUtilities::MosUnlockMutex(&mediaCtx->DecoderMutex);
    }
    if (mediaCtx->pEncoderCtxHeap != nullptr)
    {
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT pEncVACtxHeapBase = nullptr;

        MosUtilities::MosLockMutex(&mediaCtx->EncoderMutex);
        pEncVACtxHeapBase  = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)mediaCtx->pEncoderCtxHeap->pHeapBase;
        for (uint32_t j = 0; j < mediaCtx->pEncoderCtxHeap->uiAllocatedHeapElements; j++)
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
        MosUtilities::MosUnlockMutex(&mediaCtx->EncoderMutex);
    }

    return VA_STATUS_SUCCESS;
}

void MediaLibvaUtilNext::FreeSurface(DDI_MEDIA_SURFACE *surface)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surface, "nullptr surface", );
    DDI_CHK_NULL(surface->bo, "nullptr surface->bo", );
    DDI_CHK_NULL(surface->pMediaCtx, "nullptr surface->pMediaCtx", );
    DDI_CHK_NULL(surface->pMediaCtx->pGmmClientContext, "nullptr surface->pMediaCtx->pGmmClientContext", );

    // Unmap Aux mapping if the surface was mapped
    if (surface->pMediaCtx->m_auxTableMgr)
    {
        surface->pMediaCtx->m_auxTableMgr->UnmapResource(surface->pGmmResourceInfo, surface->bo);
    }

    if(surface->bMapped)
    {
        UnlockSurface(surface);
        DDI_VERBOSEMESSAGE("DDI: try to free a locked surface.");
    }
    mos_bo_unreference(surface->bo);
    // For External Buffer, only needs to destory SurfaceDescriptor
    if (surface->pSurfDesc)
    {
        MOS_FreeMemory(surface->pSurfDesc);
        surface->pSurfDesc = nullptr;
    }

    if (nullptr != surface->pGmmResourceInfo)
    {
        surface->pMediaCtx->pGmmClientContext->DestroyResInfoObject(surface->pGmmResourceInfo);
        surface->pGmmResourceInfo = nullptr;
    }
}

void MediaLibvaUtilNext::FreeBuffer(DDI_MEDIA_BUFFER  *buf)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(buf, "nullptr", );
    // calling sequence checking
    if (buf->bMapped)
    {
        UnlockBuffer(buf);
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

    if (nullptr != buf->pMediaCtx && nullptr != buf->pMediaCtx->pGmmClientContext && nullptr != buf->pGmmResourceInfo)
    {
        buf->pMediaCtx->pGmmClientContext->DestroyResInfoObject(buf->pGmmResourceInfo);
        buf->pGmmResourceInfo = nullptr;
    }
}

void* MediaLibvaUtilNext::LockSurface(DDI_MEDIA_SURFACE *surface, uint32_t flag)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surface,            "nullptr surface",            nullptr);
    DDI_CHK_NULL(surface->pMediaCtx, "nullptr surface->pMediaCtx", nullptr);

    if (MEDIA_IS_SKU(&surface->pMediaCtx->SkuTable, FtrLocalMemory))
    {
        if ((MosUtilities::MosAtomicIncrement(&surface->iRefCount) == 1) && (false == surface->bMapped))
        {
           return LockSurfaceInternal(surface, flag);
        }
        else
        {
            DDI_VERBOSEMESSAGE("line %d, invalide operation for lockSurface. the surface reference count = %d", __LINE__, surface->iRefCount);
        }
    }
    else
    {
         if ((surface->iRefCount == 0) && (false == surface->bMapped))
         {
            LockSurfaceInternal(surface, flag);
         }
         else
         {
             // do nothing here
         }
         surface->iRefCount++;
    }

    return surface->pData;
}

void* MediaLibvaUtilNext::LockSurfaceInternal(DDI_MEDIA_SURFACE  *surface, uint32_t flag)
{
    int      err      = 0;
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(surface, "nullptr surface", nullptr);
    DDI_CHK_NULL(surface->bo, "nullptr surface->bo", nullptr);
    DDI_CHK_NULL(surface->pMediaCtx, "nullptr surface->pMediaCtx", nullptr);

    // non-tield surface
    if (surface->TileType == TILING_NONE)
    {
        mos_bo_map(surface->bo, flag & MOS_LOCKFLAG_WRITEONLY);
        surface->pData     = (uint8_t*)surface->bo->virt;
        surface->data_size = surface->bo->size;
        surface->bMapped   = true;

        return surface->pData;
    }

    auto DoHwSwizzle = [&]()->bool
    {
        if (MEDIA_IS_SKU(&surface->pMediaCtx->SkuTable, FtrUseSwSwizzling))
        {
            return false;
        }
        // RGBP not supported by ve.
        if (surface->format == Media_Format_RGBP)
        {
            return false;
        }
        if (!surface->pShadowBuffer)
        {
            vaStatus = CreateShadowResource(surface);
            if (vaStatus != VA_STATUS_SUCCESS)
            {
                return false;
            }
        }

        vaStatus = SwizzleSurfaceByHW(surface);
        if (vaStatus == VA_STATUS_SUCCESS)
        {
            err = mos_bo_map(surface->pShadowBuffer->bo, flag & MOS_LOCKFLAG_WRITEONLY);
            if (err == 0)
            {
                surface->pData     = (uint8_t *)surface->pShadowBuffer->bo->virt;
                return true;
            }
        }

        // HW swizzle failed
        FreeBuffer(surface->pShadowBuffer);
        MOS_Delete(surface->pShadowBuffer);
        surface->pShadowBuffer = nullptr;
        return false;
    };

    auto DoSwSwizzle = [&]()->bool
    {
        mos_bo_map(surface->bo, flag & MOS_LOCKFLAG_WRITEONLY);

        surface->pSystemShadow = MOS_NewArray(uint8_t, surface->bo->size);
        if (!surface->pSystemShadow)
        {
            return false;
        }

        vaStatus = SwizzleSurface(surface->pMediaCtx,
                                    surface->pGmmResourceInfo,
                                    surface->bo->virt,
                                    (MOS_TILE_TYPE)surface->TileType,
                                    (uint8_t *)surface->pSystemShadow,
                                    false);
        if(vaStatus == VA_STATUS_SUCCESS)
        {
            surface->pData = surface->pSystemShadow;
            return true;
        }

        // SW swizzle failed
        MOS_DeleteArray(surface->pSystemShadow);
        surface->pSystemShadow = nullptr;
        return false;
    };

    if (!DoHwSwizzle())
    {
        if (!DoSwSwizzle())
        {
            DDI_ASSERTMESSAGE("Swizzle failed!");
            return nullptr;
        }
    }

    surface->uiMapFlag = flag;
    surface->data_size = surface->bo->size;
    surface->bMapped   = true;

    return surface->pData;
}

void MediaLibvaUtilNext::UnlockSurface(DDI_MEDIA_SURFACE  *surface)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surface, "nullptr surface", );
    DDI_CHK_NULL(surface->bo, "nullptr surface->bo", );
    if (0 == surface->iRefCount)
    {
        return;
    }

    if((true == surface->bMapped) && (1 == surface->iRefCount))
    {
        if (surface->pMediaCtx->bIsAtomSOC)
        {
            mos_bo_unmap_gtt(surface->bo);
        }
        else
        {
            if (surface->TileType == TILING_NONE)
            {
               mos_bo_unmap(surface->bo);
            }
            else if (surface->pShadowBuffer != nullptr)
            {
                SwizzleSurfaceByHW(surface, true);

                mos_bo_unmap(surface->pShadowBuffer->bo);
                FreeBuffer(surface->pShadowBuffer);
                MOS_Delete(surface->pShadowBuffer);
                surface->pShadowBuffer = nullptr;

                mos_bo_unmap(surface->bo);
            }
            else if (surface->pSystemShadow)
            {
                SwizzleSurface(surface->pMediaCtx,
                               surface->pGmmResourceInfo,
                               surface->bo->virt,
                               (MOS_TILE_TYPE)surface->TileType,
                               (uint8_t *)surface->pSystemShadow,
                               true);

                MOS_DeleteArray(surface->pSystemShadow);
                surface->pSystemShadow = nullptr;

                mos_bo_unmap(surface->bo);
            }
            else if(surface->uiMapFlag & MOS_LOCKFLAG_NO_SWIZZLE)
            {
                mos_bo_unmap(surface->bo);
            }
            else
            {
               mos_bo_unmap_gtt(surface->bo);
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

VAStatus MediaLibvaUtilNext::CreateShadowResource(DDI_MEDIA_SURFACE *surface)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(surface->pGmmResourceInfo, "nullptr surface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_SURFACE);
    if (surface->pGmmResourceInfo->GetSetCpSurfTag(0, 0) != 0)
    {
        return VA_STATUS_ERROR_INVALID_SURFACE;
    }

    if (surface->iWidth < 64 || surface->iRealHeight < 64 || (surface->iPitch % 64 != 0) || surface->format == Media_Format_P016)
    {
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    surface->pShadowBuffer = MOS_New(DDI_MEDIA_BUFFER);
    DDI_CHK_NULL(surface->pShadowBuffer, "Failed to allocate shadow buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    surface->pShadowBuffer->pMediaCtx = surface->pMediaCtx;
    surface->pShadowBuffer->bUseSysGfxMem = true;
    surface->pShadowBuffer->iSize = surface->pGmmResourceInfo->GetSizeSurface();

    vaStatus = AllocateBuffer(Media_Format_Buffer, surface->pShadowBuffer->iSize, surface->pShadowBuffer, surface->pMediaCtx->pDrmBufMgr, true);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        MOS_Delete(surface->pShadowBuffer);
        surface->pShadowBuffer = nullptr;
    }

    return vaStatus;
}

void* MediaLibvaUtilNext::LockBuffer(DDI_MEDIA_BUFFER *buf, uint32_t flag)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(buf, "nullptr buf", nullptr);
    if((Media_Format_CPU != buf->format) && (false == buf->bMapped))
    {
        if (nullptr != buf->pSurface)
        {
            LockSurface(buf->pSurface, flag);
            buf->pData = buf->pSurface->pData;
        }
        else
        {
            if (buf->pMediaCtx->bIsAtomSOC)
            {
                mos_bo_map_gtt(buf->bo);
            }
            else
            {
                if (buf->TileType == TILING_NONE)
                {
                    mos_bo_map(buf->bo, ((MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY) & flag));
                }
                else
                {
                    mos_bo_map_gtt(buf->bo);
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

void MediaLibvaUtilNext::UnlockBuffer(DDI_MEDIA_BUFFER *buf)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(buf, "nullptr buf", );
    if (0 == buf->iRefCount)
    {
        return;
    }
    if((true == buf->bMapped) && (Media_Format_CPU != buf->format) && (1 == buf->iRefCount))
    {
        if (nullptr != buf->pSurface)
        {
            UnlockSurface(buf->pSurface);
        }
        else
        {
             if (buf->pMediaCtx->bIsAtomSOC)
             {
                 mos_bo_unmap_gtt(buf->bo);
             }
             else
             {
                 if (buf->TileType == TILING_NONE)
                 {
                     mos_bo_unmap(buf->bo);
                 }
                 else
                 {
                     mos_bo_unmap_gtt(buf->bo);
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

VAStatus MediaLibvaUtilNext::SwizzleSurface(
    PDDI_MEDIA_CONTEXT         mediaCtx, 
    PGMM_RESOURCE_INFO         pGmmResInfo,
    void                       *pLockedAddr, 
    uint32_t                   TileType, 
    uint8_t                    *pResourceBase, 
    bool                       bUpload)
{
    uint32_t            uiSize         = 0, uiPitch = 0;
    GMM_RES_COPY_BLT    gmmResCopyBlt  = {0};
    uint32_t            uiPicHeight    = 0;
    uint32_t            ulSwizzledSize = 0;
    VAStatus            vaStatus       = VA_STATUS_SUCCESS;
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(pGmmResInfo,   "pGmmResInfo is NULL",   VA_STATUS_ERROR_OPERATION_FAILED);
    DDI_CHK_NULL(pLockedAddr,   "pLockedAddr is NULL",   VA_STATUS_ERROR_OPERATION_FAILED);
    DDI_CHK_NULL(pResourceBase, "pResourceBase is NULL", VA_STATUS_ERROR_ALLOCATION_FAILED);

    uiPicHeight = pGmmResInfo->GetBaseHeight();
    uiSize      = pGmmResInfo->GetSizeSurface();
    uiPitch     = pGmmResInfo->GetRenderPitch();
    gmmResCopyBlt.Gpu.pData      = pLockedAddr;
    gmmResCopyBlt.Sys.pData      = pResourceBase;
    gmmResCopyBlt.Sys.RowPitch   = uiPitch;
    gmmResCopyBlt.Sys.BufferSize = uiSize;
    gmmResCopyBlt.Sys.SlicePitch = uiSize;
    gmmResCopyBlt.Blt.Slices     = 1;
    gmmResCopyBlt.Blt.Upload     = bUpload;

    if(mediaCtx->pGmmClientContext->IsPlanar(pGmmResInfo->GetResourceFormat()) == true)
    {
        gmmResCopyBlt.Blt.Width  = pGmmResInfo->GetBaseWidth();
        gmmResCopyBlt.Blt.Height = uiSize/uiPitch;
    }

    pGmmResInfo->CpuBlt(&gmmResCopyBlt);

    return vaStatus;
}

void MediaLibvaUtilNext::ReleasePMediaBufferFromHeap(
    PDDI_MEDIA_HEAP  bufferHeap,
    uint32_t         vaBufferID)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(bufferHeap, "nullptr bufferHeap", );

    DDI_CHK_LESS(vaBufferID, bufferHeap->uiAllocatedHeapElements, "invalid buffer id", );
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapBase  =  (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)bufferHeap->pHeapBase;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapElmt  =  &mediaBufferHeapBase[vaBufferID];
    DDI_CHK_NULL(mediaBufferHeapElmt->pBuffer, "buffer is already released", );
    void *firstFree                        = bufferHeap->pFirstFreeHeapElement;
    bufferHeap->pFirstFreeHeapElement      = (void*)mediaBufferHeapElmt;
    mediaBufferHeapElmt->pNextFree         = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)firstFree;
    mediaBufferHeapElmt->pBuffer           = nullptr;
    return;
}

VAStatus MediaLibvaUtilNext::SwizzleSurfaceByHW(DDI_MEDIA_SURFACE *surface, bool isDeSwizzle)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(surface->pMediaCtx, "nullptr media context", VA_STATUS_ERROR_INVALID_CONTEXT);

    MOS_CONTEXT mosCtx = {};
    PERF_DATA perfData = {};
    PDDI_MEDIA_CONTEXT mediaDrvCtx = surface->pMediaCtx;

    // Get the buf manager for codechal create
    mosCtx.bufmgr          = mediaDrvCtx->pDrmBufMgr;
    mosCtx.fd              = mediaDrvCtx->fd;
    mosCtx.iDeviceId       = mediaDrvCtx->iDeviceId;
    mosCtx.m_skuTable        = mediaDrvCtx->SkuTable;
    mosCtx.m_waTable         = mediaDrvCtx->WaTable;
    mosCtx.m_gtSystemInfo    = *mediaDrvCtx->pGtSystemInfo;
    mosCtx.m_platform        = mediaDrvCtx->platform;

    mosCtx.ppMediaMemDecompState = &mediaDrvCtx->pMediaMemDecompState;
    mosCtx.pfnMemoryDecompress   = mediaDrvCtx->pfnMemoryDecompress;
    mosCtx.pfnMediaMemoryCopy    = mediaDrvCtx->pfnMediaMemoryCopy;
    mosCtx.pfnMediaMemoryCopy2D  = mediaDrvCtx->pfnMediaMemoryCopy2D;
    mosCtx.pPerfData             = &perfData;
    mosCtx.m_gtSystemInfo        = *mediaDrvCtx->pGtSystemInfo;
    mosCtx.m_auxTableMgr         = mediaDrvCtx->m_auxTableMgr;
    mosCtx.pGmmClientContext     = mediaDrvCtx->pGmmClientContext;

    mosCtx.m_osDeviceContext     = mediaDrvCtx->m_osDeviceContext;
    mosCtx.m_userSettingPtr      = mediaDrvCtx->m_userSettingPtr;

    MOS_RESOURCE source = {};
    MOS_RESOURCE target = {};

    if (isDeSwizzle)
    {
        MediaLibvaCommonNext::MediaBufferToMosResource(surface->pShadowBuffer, &source);
        MediaLibvaCommonNext::MediaSurfaceToMosResource(surface, &target);
    }
    else
    {
        MediaLibvaCommonNext::MediaSurfaceToMosResource(surface, &source);
        MediaLibvaCommonNext::MediaBufferToMosResource(surface->pShadowBuffer, &target);
    }

    DDI_NORMALMESSAGE("If mmd device isn't registered, use media blt copy.");
    MediaCopyBaseState *mediaCopyState = static_cast<MediaCopyBaseState*>(mediaDrvCtx->pMediaCopyState);
    if (!mediaCopyState)
    {
        mediaCopyState = static_cast<MediaCopyBaseState*>(McpyDeviceNext::CreateFactory(&mosCtx));
        if (!mediaCopyState)
        {
            return VA_STATUS_ERROR_UNKNOWN;
        }
        mediaDrvCtx->pMediaCopyState = mediaCopyState;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // disable reg key report to avoid conflict with media copy cases.
    mediaCopyState->SetRegkeyReport(false);
#endif

    auto format = surface->pGmmResourceInfo->GetResourceFormat();
    auto width  = surface->pGmmResourceInfo->GetBaseWidth();
    auto height = surface->pGmmResourceInfo->GetBaseHeight();
    auto pitch  = surface->pGmmResourceInfo->GetRenderPitch();

    auto uOffsetX = surface->pGmmResourceInfo->GetPlanarXOffset(GMM_PLANE_U);
    auto uOffsetY = surface->pGmmResourceInfo->GetPlanarYOffset(GMM_PLANE_U);
    auto vOffsetX = surface->pGmmResourceInfo->GetPlanarXOffset(GMM_PLANE_V);
    auto vOffsetY = surface->pGmmResourceInfo->GetPlanarYOffset(GMM_PLANE_V);

    DDI_NORMALMESSAGE("Override param: format %d, width %d, height %d, pitch %d", format, width, height, pitch);

    DDI_CHK_NULL(source.pGmmResInfo, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(target.pGmmResInfo, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);

    if (isDeSwizzle)
    {
        source.pGmmResInfo->OverrideSurfaceFormat(format);
        source.pGmmResInfo->OverrideSurfaceType(RESOURCE_2D);
        source.pGmmResInfo->OverrideBaseWidth(width);
        source.pGmmResInfo->OverrideBaseHeight(height);
        source.pGmmResInfo->OverridePitch(pitch);

        source.pGmmResInfo->OverridePlanarXOffset(GMM_PLANE_U, uOffsetX);
        source.pGmmResInfo->OverridePlanarYOffset(GMM_PLANE_U, uOffsetY);
        source.pGmmResInfo->OverridePlanarXOffset(GMM_PLANE_V, vOffsetX);
        source.pGmmResInfo->OverridePlanarYOffset(GMM_PLANE_V, vOffsetY);

        source.Format = target.Format;
    }
    else
    {
        target.pGmmResInfo->OverrideSurfaceFormat(format);
        target.pGmmResInfo->OverrideSurfaceType(RESOURCE_2D);
        target.pGmmResInfo->OverrideBaseWidth(width);
        target.pGmmResInfo->OverrideBaseHeight(height);
        target.pGmmResInfo->OverridePitch(pitch);

        target.pGmmResInfo->OverridePlanarXOffset(GMM_PLANE_U, uOffsetX);
        target.pGmmResInfo->OverridePlanarYOffset(GMM_PLANE_U, uOffsetY);
        target.pGmmResInfo->OverridePlanarXOffset(GMM_PLANE_V, vOffsetX);
        target.pGmmResInfo->OverridePlanarYOffset(GMM_PLANE_V, vOffsetY);

        target.Format = source.Format;
    }

    MOS_STATUS mosSts = mediaCopyState->SurfaceCopy(&source, &target, MCPY_METHOD_BALANCE);

    if (mosSts == MOS_STATUS_SUCCESS)
    {
        return VA_STATUS_SUCCESS;
    }
    else
    {
        return VA_STATUS_ERROR_UNKNOWN;
    }
}

VAStatus MediaLibvaUtilNext::CreateBuffer(
    DDI_MEDIA_BUFFER *buffer,
    MOS_BUFMGR       *bufmgr)
{
    VAStatus status = VA_STATUS_SUCCESS;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(buffer,                             "nullptr buffer",         VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_LESS(buffer->format, Media_Format_Count, "Invalid buffer->format", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (buffer->format == Media_Format_CPU)
    {
        buffer->pData= (uint8_t*)MOS_AllocAndZeroMemory(buffer->iSize);
        if (nullptr == buffer->pData)
        {
            status = VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }
    else
    {
        if (Media_Format_2DBuffer == buffer->format)
        {
            status = Allocate2DBuffer(buffer->uiHeight, buffer->uiWidth, buffer, bufmgr);
        }
        else
        {
            status = AllocateBuffer(buffer->format, buffer->iSize, buffer, bufmgr);
        }
    }

    buffer->uiLockedBufID   = VA_INVALID_ID;
    buffer->uiLockedImageID = VA_INVALID_ID;
    buffer->iRefCount       = 0;

    return status;
}

VAStatus MediaLibvaUtilNext::Allocate2DBuffer(
    uint32_t             height,
    uint32_t             width,
    PDDI_MEDIA_BUFFER    mediaBuffer,
    MOS_BUFMGR           *bufmgr)
{
    DDI_CHK_NULL(mediaBuffer,                               "mediaBuffer is nullptr",                               VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaBuffer->pMediaCtx,                    "mediaBuffer->pMediaCtx is nullptr",                    VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaBuffer->pMediaCtx->pGmmClientContext, "mediaBuffer->pMediaCtx->pGmmClientContext is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);

    int32_t  size           = 0;
    uint32_t tileformat     = TILING_NONE;
    VAStatus hRes           = VA_STATUS_SUCCESS;
    int32_t  mem_type       = MOS_MEMPOOL_VIDEOMEMORY;

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
    gmmParams.Flags.Info.LocalOnly = MEDIA_IS_SKU(&mediaBuffer->pMediaCtx->SkuTable, FtrLocalMemory);
    GMM_RESOURCE_INFO  *gmmResourceInfo;
    mediaBuffer->pGmmResourceInfo = gmmResourceInfo = mediaBuffer->pMediaCtx->pGmmClientContext->CreateResInfoObject(&gmmParams);

    if(nullptr == gmmResourceInfo)
    {
        DDI_VERBOSEMESSAGE("Gmm Create Resource Failed.");
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return hRes;
    }
    uint32_t    gmmPitch;
    uint32_t    gmmSize;
    uint32_t    gmmHeight;
    gmmPitch    = (uint32_t)gmmResourceInfo->GetRenderPitch();
    gmmSize     = (uint32_t)gmmResourceInfo->GetSizeSurface();
    gmmHeight   = gmmResourceInfo->GetBaseHeight();

    MemoryPolicyParameter memPolicyPar = { 0 };
    memPolicyPar.skuTable = &mediaBuffer->pMediaCtx->SkuTable;
    memPolicyPar.waTable  = &mediaBuffer->pMediaCtx->WaTable;
    memPolicyPar.resInfo  = mediaBuffer->pGmmResourceInfo;
    memPolicyPar.resName  = "Media 2D Buffer";
    memPolicyPar.uiType   = mediaBuffer->uiType;
    memPolicyPar.preferredMemType = mediaBuffer->bUseSysGfxMem ? MOS_MEMPOOL_SYSTEMMEMORY : 0;

    mem_type = MemoryPolicyManager::UpdateMemoryPolicy(&memPolicyPar);

    unsigned int patIndex = MosInterface::GetPATIndexFromGmm(mediaBuffer->pMediaCtx->pGmmClientContext, gmmResourceInfo);
    bool isCpuCacheable   = gmmResourceInfo->GetResFlags().Info.Cacheable;

    MOS_LINUX_BO  *bo;
    struct mos_drm_bo_alloc alloc;
    alloc.name = "Media 2D Buffer";
    alloc.size = gmmSize;
    alloc.alignment = 4096;
    alloc.ext.tiling_mode = TILING_NONE;
    alloc.ext.mem_type = mem_type;
    alloc.ext.pat_index = patIndex;
    alloc.ext.cpu_cacheable = isCpuCacheable;

    bo = mos_bo_alloc(bufmgr, &alloc);

    mediaBuffer->bMapped = false;
    if (bo)
    {
        mediaBuffer->format     = Media_Format_2DBuffer;
        mediaBuffer->uiWidth    = width;
        mediaBuffer->uiHeight   = gmmHeight;
        mediaBuffer->uiPitch    = gmmPitch;
        mediaBuffer->iSize      = gmmSize;
        mediaBuffer->iRefCount  = 0;
        mediaBuffer->bo         = bo;
        mediaBuffer->TileType   = tileformat;
        mediaBuffer->pData      = (uint8_t*) bo->virt;
        DDI_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource)\n", size, width, height);
        uint32_t event[] = {bo->handle, mediaBuffer->format, width, height, gmmPitch, bo->size, tileformat, 0};
        MOS_TraceEventExt(EVENT_VA_BUFFER, EVENT_TYPE_INFO, event, sizeof(event), &gmmResourceInfo->GetResFlags(), sizeof(GMM_RESOURCE_FLAG));
    }
    else
    {
        DDI_VERBOSEMESSAGE("Fail to Alloc %7d bytes (%d x %d resource)\n", size, width, height);
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    return hRes;
}

VAStatus MediaLibvaUtilNext::AllocateBuffer(
    DDI_MEDIA_FORMAT     format,
    int32_t              size,
    PDDI_MEDIA_BUFFER    mediaBuffer,
    MOS_BUFMGR           *bufmgr,
    bool                 isShadowBuffer)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaBuffer,                               "mediaBuffer is nullptr",                               VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaBuffer->pMediaCtx,                    "mediaBuffer->pMediaCtx is nullptr",                    VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(mediaBuffer->pMediaCtx->pGmmClientContext, "mediaBuffer->pMediaCtx->pGmmClientContext is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    if(format >= Media_Format_Count)
    {
       return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    VAStatus     hRes = VA_STATUS_SUCCESS;
    int32_t      mem_type = MOS_MEMPOOL_VIDEOMEMORY;

    // create fake GmmResourceInfo
    GMM_RESCREATE_PARAMS gmmParams;
    MOS_ZeroMemory(&gmmParams, sizeof(gmmParams));
    gmmParams.BaseWidth             = 1;
    gmmParams.BaseHeight            = 1;
    gmmParams.ArraySize             = 0;
    gmmParams.Type                  = RESOURCE_1D;
    gmmParams.Format                = GMM_FORMAT_GENERIC_8BIT;
    gmmParams.Flags.Gpu.Video       = true;
    gmmParams.Flags.Info.Linear     = true;
    gmmParams.Flags.Info.LocalOnly  = MEDIA_IS_SKU(&mediaBuffer->pMediaCtx->SkuTable, FtrLocalMemory);

    if (isShadowBuffer)
    {
        gmmParams.Flags.Info.Cacheable = true;
        gmmParams.Usage = GMM_RESOURCE_USAGE_STAGING;
    }

    mediaBuffer->pGmmResourceInfo = mediaBuffer->pMediaCtx->pGmmClientContext->CreateResInfoObject(&gmmParams);
    DDI_CHK_NULL(mediaBuffer->pGmmResourceInfo, "pGmmResourceInfo is nullptr", VA_STATUS_ERROR_INVALID_BUFFER);
    mediaBuffer->pGmmResourceInfo->OverrideSize(mediaBuffer->iSize);
    mediaBuffer->pGmmResourceInfo->OverrideBaseWidth(mediaBuffer->iSize);
    mediaBuffer->pGmmResourceInfo->OverridePitch(mediaBuffer->iSize);

    MemoryPolicyParameter memPolicyPar = { 0 };
    memPolicyPar.skuTable = &mediaBuffer->pMediaCtx->SkuTable;
    memPolicyPar.waTable  = &mediaBuffer->pMediaCtx->WaTable;
    memPolicyPar.resInfo  = mediaBuffer->pGmmResourceInfo;
    memPolicyPar.resName  = "Media Buffer";
    memPolicyPar.uiType   = mediaBuffer->uiType;
    memPolicyPar.preferredMemType = mediaBuffer->bUseSysGfxMem ? MOS_MEMPOOL_SYSTEMMEMORY : 0;

    mem_type = MemoryPolicyManager::UpdateMemoryPolicy(&memPolicyPar);

    unsigned int patIndex = MosInterface::GetPATIndexFromGmm(mediaBuffer->pMediaCtx->pGmmClientContext, mediaBuffer->pGmmResourceInfo);
    bool isCpuCacheable   = mediaBuffer->pGmmResourceInfo->GetResFlags().Info.Cacheable;

    struct mos_drm_bo_alloc alloc;
    alloc.name = "Media Buffer";
    alloc.size = size;
    alloc.alignment = 4096;
    alloc.ext.tiling_mode = TILING_NONE;
    alloc.ext.mem_type = mem_type;
    alloc.ext.pat_index = patIndex;
    alloc.ext.cpu_cacheable = isCpuCacheable;

    MOS_LINUX_BO *bo  = mos_bo_alloc(bufmgr, &alloc);
    mediaBuffer->bMapped = false;
    if (bo)
    {
        mediaBuffer->format     = format;
        mediaBuffer->iSize      = size;
        mediaBuffer->iRefCount  = 0;
        mediaBuffer->bo         = bo;
        mediaBuffer->pData      = (uint8_t*) bo->virt;

        DDI_VERBOSEMESSAGE("Alloc %8d bytes resource.",size);
        uint32_t event[] = {bo->handle, format, size, 1, size, bo->size, 0, 0};
        MOS_TraceEventExt(EVENT_VA_BUFFER, EVENT_TYPE_INFO, event, sizeof(event), &mediaBuffer->pGmmResourceInfo->GetResFlags(), sizeof(GMM_RESOURCE_FLAG));
    }
    else
    {
        DDI_ASSERTMESSAGE("Fail to Alloc %8d bytes resource.",size);
        hRes = VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    return hRes;
}

PDDI_MEDIA_BUFFER_HEAP_ELEMENT MediaLibvaUtilNext::AllocPMediaBufferFromHeap(PDDI_MEDIA_HEAP bufferHeap)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(bufferHeap, "nullptr bufferHeap", nullptr);

    PDDI_MEDIA_BUFFER_HEAP_ELEMENT  mediaBufferHeapElmt = nullptr;
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

PDDI_MEDIA_IMAGE_HEAP_ELEMENT MediaLibvaUtilNext::AllocPVAImageFromHeap(PDDI_MEDIA_HEAP imageHeap)
{
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT   vaimageHeapElmt = nullptr;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(imageHeap, "nullptr imageHeap", nullptr);

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

GMM_RESOURCE_FORMAT MediaLibvaUtilNext::ConvertFourccToGmmFmt(uint32_t fourcc)
{
    DDI_FUNC_ENTER;
    switch (fourcc)
    {
        case VA_FOURCC_BGRA   : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case VA_FOURCC_ARGB   : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case VA_FOURCC_RGBA   : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case VA_FOURCC_ABGR   : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case VA_FOURCC_BGRX   : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case VA_FOURCC_XRGB   : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case VA_FOURCC_RGBX   : return GMM_FORMAT_R8G8B8X8_UNORM_TYPE;
        case VA_FOURCC_XBGR   : return GMM_FORMAT_R8G8B8X8_UNORM_TYPE;
        case VA_FOURCC_R8G8B8 : return GMM_FORMAT_R8G8B8_UNORM;
        case VA_FOURCC_RGBP   : return GMM_FORMAT_RGBP;
        case VA_FOURCC_BGRP   : return GMM_FORMAT_BGRP;
        case VA_FOURCC_RGB565 : return GMM_FORMAT_B5G6R5_UNORM_TYPE;
        case VA_FOURCC_AYUV   : return GMM_FORMAT_AYUV_TYPE;
#if VA_CHECK_VERSION(1, 13, 0)
        case VA_FOURCC_XYUV   : return GMM_FORMAT_AYUV_TYPE;
#endif
        case VA_FOURCC_NV12   : return GMM_FORMAT_NV12_TYPE;
        case VA_FOURCC_NV21   : return GMM_FORMAT_NV21_TYPE;
        case VA_FOURCC_YUY2   : return GMM_FORMAT_YUY2;
        case VA_FOURCC_UYVY   : return GMM_FORMAT_UYVY;
        case VA_FOURCC_YV12   : return GMM_FORMAT_YV12_TYPE;
        case VA_FOURCC_I420   : return GMM_FORMAT_I420_TYPE;
        case VA_FOURCC_IYUV   : return GMM_FORMAT_IYUV_TYPE;
        case VA_FOURCC_411P   : return GMM_FORMAT_MFX_JPEG_YUV411_TYPE;
        case VA_FOURCC_422H   : return GMM_FORMAT_MFX_JPEG_YUV422H_TYPE;
        case VA_FOURCC_422V   : return GMM_FORMAT_MFX_JPEG_YUV422V_TYPE;
        case VA_FOURCC_444P   : return GMM_FORMAT_MFX_JPEG_YUV444_TYPE;
        case VA_FOURCC_IMC3   : return GMM_FORMAT_IMC3_TYPE;
        case VA_FOURCC_P208   : return GMM_FORMAT_P208_TYPE;
        case VA_FOURCC_P010   : return GMM_FORMAT_P010_TYPE;
        case VA_FOURCC_P012   : return GMM_FORMAT_P016_TYPE;
        case VA_FOURCC_P016   : return GMM_FORMAT_P016_TYPE;
        case VA_FOURCC_Y210   : return GMM_FORMAT_Y210_TYPE;
        case VA_FOURCC_Y410   : return GMM_FORMAT_Y410_TYPE;
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y212   : return GMM_FORMAT_Y212_TYPE;
#endif
        case VA_FOURCC_Y216   : return GMM_FORMAT_Y216_TYPE;
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y412   : return GMM_FORMAT_Y412_TYPE;
#endif
        case VA_FOURCC_Y416   : return GMM_FORMAT_Y416_TYPE;
        case VA_FOURCC_Y800   : return GMM_FORMAT_GENERIC_8BIT;
        case VA_FOURCC_A2R10G10B10   : return GMM_FORMAT_R10G10B10A2_UNORM_TYPE;
        case VA_FOURCC_A2B10G10R10   : return GMM_FORMAT_B10G10R10A2_UNORM_TYPE;
        case VA_FOURCC_X2R10G10B10   : return GMM_FORMAT_R10G10B10A2_UNORM_TYPE;
        case VA_FOURCC_X2B10G10R10   : return GMM_FORMAT_B10G10R10A2_UNORM_TYPE;
        default               : return GMM_FORMAT_INVALID;
    }
}

void MediaLibvaUtilNext::InitMutex(PMEDIA_MUTEX_T mutex)
{
    pthread_mutex_init(mutex, nullptr);
}

void MediaLibvaUtilNext::DestroyMutex(PMEDIA_MUTEX_T mutex)
{
    int32_t ret = pthread_mutex_destroy(mutex);
    if(ret != 0)
    {
        DDI_NORMALMESSAGE("can't destroy the mutex!\n");
    }
}

VAStatus MediaLibvaUtilNext::SetMediaResetEnableFlag(PDDI_MEDIA_CONTEXT mediaCtx)
{
    bool enableReset = false;
    mediaCtx->bMediaResetEnable = false;

    DDI_CHK_NULL(mediaCtx,"nullptr mediaCtx!", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(!MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrSWMediaReset))
    {
        mediaCtx->bMediaResetEnable = false;
        return VA_STATUS_SUCCESS;
    }

    mediaCtx->bMediaResetEnable = true;

#if (_DEBUG || _RELEASE_INTERNAL)
    ReadUserSettingForDebug(
        mediaCtx->m_userSettingPtr,
        enableReset,
        __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_ENABLE,
        MediaUserSetting::Group::Device);
    mediaCtx->bMediaResetEnable = enableReset;
#endif
    if(!mediaCtx->bMediaResetEnable)
    {
        return VA_STATUS_SUCCESS;
    }

    char* mediaResetEnv = getenv("INTEL_MEDIA_RESET_WATCHDOG");
    if(mediaResetEnv)
    {
        mediaCtx->bMediaResetEnable = strcmp(mediaResetEnv, "1") ? false : true;
        return VA_STATUS_SUCCESS;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaUtilNext::GetSurfaceModifier(
    DDI_MEDIA_CONTEXT  *mediaCtx,
    DDI_MEDIA_SURFACE  *mediaSurface,
    uint64_t           &modifier)
{
    DDI_CHK_NULL(mediaCtx,                       "nullptr media context",                  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaSurface,                   "nullptr mediaSurface",                   VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->bo,               "nullptr mediaSurface->bo",               VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->pGmmResourceInfo, "nullptr mediaSurface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_SURFACE);
    GMM_TILE_TYPE  gmmTileType = mediaSurface->pGmmResourceInfo->GetTileType();
    GMM_RESOURCE_FLAG       gmmFlags    = {0};
    gmmFlags = mediaSurface->pGmmResourceInfo->GetResFlags();

    if(MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrXe2Compression))
    {
        // Update Xe2Compression Modifier
        uint64_t compressedModifier = I915_FORMAT_MOD_4_TILED_LNL_CCS;

        if (MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrLocalMemory))
        {
            // DGfx only support compression on Local memory
            compressedModifier = I915_FORMAT_MOD_4_TILED_BMG_CCS;
        }
        switch(gmmTileType)
        {
            case GMM_TILED_4:
                modifier = gmmFlags.Info.MediaCompressed ? compressedModifier : I915_FORMAT_MOD_4_TILED;
                break;
            case GMM_TILED_Y:
                modifier = I915_FORMAT_MOD_Y_TILED;
            break;
            case GMM_TILED_X:
                modifier = I915_FORMAT_MOD_X_TILED;
                break;
            case GMM_NOT_TILED:
                modifier = DRM_FORMAT_MOD_LINEAR;
                break;
            default:
                //handle other possible tile format
                if(TILING_Y == mediaSurface->TileType)
                {
                    modifier = gmmFlags.Info.MediaCompressed ? compressedModifier : I915_FORMAT_MOD_4_TILED;
                }
                else
                {
                    modifier = DRM_FORMAT_MOD_LINEAR;
                }
                break;
        }

        return VA_STATUS_SUCCESS;
    }

    bool                    bMmcEnabled = false;
    if ((gmmFlags.Gpu.MMC               ||
         gmmFlags.Gpu.CCS)              &&
        (gmmFlags.Info.MediaCompressed  ||
         gmmFlags.Info.RenderCompressed))
    {
        bMmcEnabled = true;
    }
    else
    {
        bMmcEnabled = false;
    }

    switch(gmmTileType)
    {
        case GMM_TILED_4:
            if(mediaCtx->m_auxTableMgr && bMmcEnabled)
            {
                modifier = gmmFlags.Info.MediaCompressed ? I915_FORMAT_MOD_4_TILED_MTL_MC_CCS :
                    (gmmFlags.Info.RenderCompressed ? I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC : I915_FORMAT_MOD_4_TILED);
            }
            else
            {
                modifier = I915_FORMAT_MOD_4_TILED;
            }
            break;
        case GMM_TILED_Y:
            if (mediaCtx->m_auxTableMgr && bMmcEnabled)
            {
                modifier = gmmFlags.Info.MediaCompressed ? I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS :
                    (gmmFlags.Info.RenderCompressed ? I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS : I915_FORMAT_MOD_Y_TILED);
            }
            else
            {
                modifier = I915_FORMAT_MOD_Y_TILED;
            }
            break;
        case GMM_TILED_X:
            modifier = I915_FORMAT_MOD_X_TILED;
            break;
        case GMM_NOT_TILED:
            modifier = DRM_FORMAT_MOD_LINEAR;
            break;
        default:
            //handle other possible tile format
            if(TILING_Y == mediaSurface->TileType)
            {
                modifier = gmmFlags.Info.MediaCompressed ? I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS :
                    (gmmFlags.Info.RenderCompressed ? I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS : I915_FORMAT_MOD_Y_TILED);
            }
            else
            {
                modifier = DRM_FORMAT_MOD_LINEAR;
            }
            break;

    }
    return VA_STATUS_SUCCESS;
}

PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT MediaLibvaUtilNext::DdiAllocPVAContextFromHeap(
    PDDI_MEDIA_HEAP vaContextHeap)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT  vacontextHeapElmt = nullptr;
    DDI_FUNCTION_ENTER();
    DDI_CHK_NULL(vaContextHeap, "nullptr vaContextHeap", nullptr);

    if (nullptr == vaContextHeap->pFirstFreeHeapElement)
    {
        void *newHeapBase = MOS_ReallocMemory(vaContextHeap->pHeapBase, (vaContextHeap->uiAllocatedHeapElements + DDI_MEDIA_HEAP_INCREMENTAL_SIZE) * sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT));
        DDI_CHK_NULL(newHeapBase, "DDI: realloc failed.", nullptr);

        vaContextHeap->pHeapBase                            = newHeapBase;
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vacontextHeapBase = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)vaContextHeap->pHeapBase;
        DDI_CHK_NULL(vacontextHeapBase, "nullptr vacontextHeapBase.", nullptr);
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

    vacontextHeapElmt                    = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)vaContextHeap->pFirstFreeHeapElement;
    vaContextHeap->pFirstFreeHeapElement = vacontextHeapElmt->pNextFree;
    return vacontextHeapElmt;
}

void MediaLibvaUtilNext::DdiReleasePVAContextFromHeap(
    PDDI_MEDIA_HEAP vaContextHeap,
    uint32_t        vaContextID)
{
    DDI_FUNCTION_ENTER();
    DDI_CHK_NULL(vaContextHeap, "nullptr vaContextHeap", );
    DDI_CHK_LESS(vaContextID, vaContextHeap->uiAllocatedHeapElements, "invalid context id", );

    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaContextHeapBase = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)vaContextHeap->pHeapBase;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaContextHeapElmt = &vaContextHeapBase[vaContextID];
    DDI_CHK_NULL(vaContextHeapElmt->pVaContext, "context is already released", );
    void *firstFree                        = vaContextHeap->pFirstFreeHeapElement;

    vaContextHeap->pFirstFreeHeapElement   = (void*)vaContextHeapElmt;
    vaContextHeapElmt->pNextFree           = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)firstFree;
    vaContextHeapElmt->pVaContext          = nullptr;

    return;
}

MOS_FORMAT MediaLibvaUtilNext::GetFormatFromMediaFormat(DDI_MEDIA_FORMAT mediaFormat)
{
    MOS_FORMAT format = Format_Invalid;
    DDI_FUNC_ENTER;
    static const std::map<const DDI_MEDIA_FORMAT, const MOS_FORMAT> ddiFormatToMediaFormatMap =
    {
        {Media_Format_NV12, Format_NV12},
        {Media_Format_NV21, Format_NV21},
        {Media_Format_X8R8G8B8, Format_X8R8G8B8},
        {Media_Format_X8B8G8R8, Format_X8B8G8R8},
        {Media_Format_A8R8G8B8, Format_A8R8G8B8},
        {Media_Format_A8B8G8R8, Format_A8B8G8R8},
        {Media_Format_R8G8B8A8, Format_A8B8G8R8},
        {Media_Format_R5G6B5, Format_R5G6B5},
        {Media_Format_R8G8B8, Format_R8G8B8},
        {Media_Format_YUY2, Format_YUY2},
        {Media_Format_UYVY, Format_UYVY},
        {Media_Format_YV12, Format_YV12},
        {Media_Format_I420, Format_I420},
        {Media_Format_IYUV, Format_IYUV},
        {Media_Format_422H, Format_422H},
        {Media_Format_422V, Format_422V},
        {Media_Format_400P, Format_400P},
        {Media_Format_411P, Format_411P},
        {Media_Format_444P, Format_444P},
        {Media_Format_IMC3, Format_IMC3},
        {Media_Format_P010, Format_P010},
        {Media_Format_P012, Format_P016},
        {Media_Format_P016, Format_P016},
        {Media_Format_R10G10B10A2, Format_R10G10B10A2},
        {Media_Format_R10G10B10X2, Format_R10G10B10A2},
        {Media_Format_B10G10R10A2, Format_B10G10R10A2},
        {Media_Format_B10G10R10X2, Format_B10G10R10A2},
        {Media_Format_RGBP, Format_RGBP},
        {Media_Format_BGRP, Format_BGRP},
        {Media_Format_Y210, Format_Y210},
#if VA_CHECK_VERSION(1, 9, 0)
        {Media_Format_Y212, Format_Y216},
        {Media_Format_Y412, Format_Y416},
#endif
        {Media_Format_Y216, Format_Y216},
        {Media_Format_Y410, Format_Y410},
        {Media_Format_Y416, Format_Y416},
        {Media_Format_AYUV, Format_AYUV},
#if VA_CHECK_VERSION(1, 13, 0)
        {Media_Format_XYUV, Format_AYUV},
#endif
        {Media_Format_Y8, Format_Y8},
        {Media_Format_Y16S, Format_Y16S},
        {Media_Format_Y16U, Format_Y16U},
        {Media_Format_VYUY, Format_VYUY},
        {Media_Format_YVYU, Format_YVYU},
        {Media_Format_A16R16G16B16, Format_A16R16G16B16},
        {Media_Format_A16B16G16R16, Format_A16B16G16R16}
    };
    auto it = ddiFormatToMediaFormatMap.find(mediaFormat);
    if(it != ddiFormatToMediaFormatMap.end())
    {
        return it->second;
    }
    else
    {
        DDI_ASSERTMESSAGE("ERROR media format to vphal format.");
        return Format_Invalid;
    }
}

MOS_TILE_TYPE MediaLibvaUtilNext::GetTileTypeFromMediaTileType(uint32_t mediaTileType)
{
    MOS_TILE_TYPE tileType = MOS_TILE_INVALID;
    DDI_FUNC_ENTER;

    switch(mediaTileType)
    {
       case TILING_Y:
           tileType = MOS_TILE_Y;
           break;
       case TILING_X:
           tileType = MOS_TILE_X;
           break;
       case TILING_NONE:
           tileType = MOS_TILE_LINEAR;
           break;
        default:
           tileType = MOS_TILE_LINEAR;
    }

    return tileType;
}

VPHAL_CSPACE MediaLibvaUtilNext::GetColorSpaceFromMediaFormat(DDI_MEDIA_FORMAT format)
{
    DDI_FUNC_ENTER;
    MOS_FORMAT mosFormat = GetFormatFromMediaFormat(format);

    if (IS_RGB_FORMAT(mosFormat))
    {
        return CSpace_sRGB;
    }
    else
    {
        return CSpace_BT601;
    }
}

void MediaLibvaUtilNext::UnRefBufObjInMediaBuffer(PDDI_MEDIA_BUFFER buf)
{
    DDI_FUNC_ENTER;
    mos_bo_unreference(buf->bo);
    return;
}

void MediaLibvaUtilNext::ReleasePVAImageFromHeap(PDDI_MEDIA_HEAP imageHeap, uint32_t vaImageID)
{
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT    vaImageHeapBase = nullptr;
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT    vaImageHeapElmt = nullptr;
    void                             *firstFree      = nullptr;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(imageHeap, "nullptr imageHeap", );

    DDI_CHK_LESS(vaImageID, imageHeap->uiAllocatedHeapElements, "invalid image id", );
    vaImageHeapBase                    = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)imageHeap->pHeapBase;
    vaImageHeapElmt                    = &vaImageHeapBase[vaImageID];
    DDI_CHK_NULL(vaImageHeapElmt->pImage, "image is already released", );
    firstFree                          = imageHeap->pFirstFreeHeapElement;
    imageHeap->pFirstFreeHeapElement   = (void*)vaImageHeapElmt;
    vaImageHeapElmt->pNextFree         = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)firstFree;
    vaImageHeapElmt->pImage            = nullptr;
}

#ifdef RELEASE
void MediaLibvaUtilNext::MediaPrintFps()
{
    return;
}
#else
void MediaLibvaUtilNext::MediaPrintFps()
{
    struct timeval tv2 ={};
    DDI_FUNC_ENTER;
    if (m_isMediaFpsPrintFpsEnabled == false)
    {
        return;
    }
    if (0 == m_vaFpsSampleSize)
    {
        return;
    }
    gettimeofday(&tv2, 0);

    pthread_mutex_lock(&m_fpsMutex);
    if (-1 == m_frameCountFps)
    {
        gettimeofday(&m_tv1, 0);
    }

    if (++m_frameCountFps >= m_vaFpsSampleSize)
    {
        char   fpsFileName[LENGTH_OF_FPS_FILE_NAME] = {};
        FILE   *fp                                  = nullptr;
        char   temp[LENGTH_OF_FPS_FILE_NAME]        = {};

        int64_t diff  = (tv2.tv_sec - m_tv1.tv_sec)*1000000 + tv2.tv_usec - m_tv1.tv_usec;
        float fps     = m_frameCountFps / (diff / 1000000.0);
        DDI_NORMALMESSAGE("FPS:%6.4f, Interval:%11lu.", fps,((uint64_t)tv2.tv_sec)*1000 + (tv2.tv_usec/1000));
        sprintf_s(temp, sizeof(temp), "FPS:%6.4f, Interval:%" PRIu64"\n", fps,((uint64_t)tv2.tv_sec)*1000 + (tv2.tv_usec/1000));

        MOS_ZeroMemory(fpsFileName,LENGTH_OF_FPS_FILE_NAME);
        sprintf_s(fpsFileName, sizeof(fpsFileName), FPS_FILE_NAME);
        if ((fp = fopen(fpsFileName, "wb")) == nullptr)
        {
            pthread_mutex_unlock(&m_fpsMutex);
            DDI_ASSERTMESSAGE("Unable to open fps file.");
            return;
        }

        fwrite(temp, 1, strlen(temp), fp);
        fclose(fp);
        m_frameCountFps = -1;
    }
    pthread_mutex_unlock(&m_fpsMutex);
    return;
}
#endif
