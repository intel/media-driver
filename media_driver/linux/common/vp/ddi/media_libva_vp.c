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
//! \file     media_libva_vp.c
//! \brief    LibVA Video Processing extension interface implementation
//!

#include <va/va.h>
#include <va/va_vpp.h>
#include <va/va_backend.h>

#include <dlfcn.h>
#include <limits.h>

#include "vphal_ddi.h"

#include "media_libva.h"
#include "media_libva_vp.h"
#include "media_libva_util.h"
#include "hwinfo_linux.h"
#include "mos_solo_generic.h"

#if (_DEBUG || _RELEASE_INTERNAL)
#include "media_libva_vp_tools.h"
#if ANDROID
#include "media_libva_vp_tools_android.h"
#endif
#endif

#define VP_SETTING_MAX_PHASES                           1
#define VP_SETTING_MEDIA_STATES                         32
#define VP_SETTING_SAME_SAMPLE_THRESHOLD                1000

VAStatus DdiMedia_MapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id,
    void                **pbuf
);
VAStatus DdiMedia_UnMapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id
);

// VP internal APIs to access VP acceleration capability in VPG drivers
VAStatus     DdiVp_InitVpHal(PDDI_VP_CONTEXT);
VAStatus     DdiVp_DestroyVpHal(PDDI_VP_CONTEXT);
VAStatus     DdiVp_DestroyVpHalSurface(PVPHAL_SURFACE pSurf);
VAStatus     DdiVp_DestroySrcParams(PDDI_VP_CONTEXT pVpCtx);
VAStatus     DdiVp_DestroyTargetParams(PDDI_VP_CONTEXT pVpCtx);
VAStatus     DdiVp_DestroyRenderParams(PDDI_VP_CONTEXT pVpCtx);
#ifdef ANDROID
VPHAL_CSPACE DdiVp_GetColorSpace(VAProcColorStandardType, uint32_t);
#else
VPHAL_CSPACE DdiVp_GetColorSpace(VAProcColorStandardType, uint8_t);
#endif
VAStatus     DdiVp_UpdateVphalTargetSurfColorSpace(VADriverContextP, PDDI_VP_CONTEXT, VAProcPipelineParameterBuffer*);
			
VAStatus     DdiVp_InitCtx(VADriverContextP, PDDI_VP_CONTEXT);
VAStatus     DdiVp_SetProcPipelineParams(VADriverContextP, PDDI_VP_CONTEXT, VAProcPipelineParameterBuffer*);
VAStatus     DdiVp_UpdateFilterParamBuffer(PDDI_VP_CONTEXT, uint32_t, int32_t, void *, uint32_t, DDI_VP_STATE*);
VAStatus     DdiVp_ClearFilterParamBuffer(PDDI_VP_CONTEXT , uint32_t, DDI_VP_STATE);
VAStatus     DdiVp_SetProcFilterDinterlaceParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBufferDeinterlacing*);
VAStatus     DdiVp_SetProcFilterDenoiseParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBuffer*);
VAStatus     DdiVp_SetProcFilterSharpnessParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBuffer*);
VAStatus     DdiVp_SetProcFilterColorBalanceParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBufferColorBalance*, uint32_t );
VAStatus     DdiVp_SetProcFilterSkinToneEnhancementParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBuffer*);
VAStatus     DdiVp_SetProcFilterTotalColorCorrectionParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBufferTotalColorCorrection*, uint32_t);
//VAStatus     DdiVp_UpdateProcDeinterlaceParams(VADriverContextP pVaDrvCtx, PVPHAL_SURFACE  pVpHalSrcSurf, VAProcPipelineParameterBuffer*  pPipelineParam);
VAStatus     DdiVp_SetProcPipelineBlendingParams(PDDI_VP_CONTEXT pVpCtx, uint32_t uiSurfIndex, VAProcPipelineParameterBuffer* pPipelineParam);
VAStatus     DdiVp_ConvertSurface (VADriverContextP ctx, DDI_MEDIA_SURFACE  *srcSurface, int16_t srcx,  int16_t srcy, uint16_t srcw,  uint16_t srch,  DDI_MEDIA_SURFACE  *dstSurface,  int16_t destx,  int16_t desty, uint16_t destw, uint16_t desth );
VAStatus     DdiVp_UpdateProcPipelineForwardReferenceFrames(PDDI_VP_CONTEXT pVpCtx, VADriverContextP pVaDrvCtx, PVPHAL_SURFACE pVpHalSrcSurf, VAProcPipelineParameterBuffer* pPipelineParam);
VAStatus     DdiVp_UpdateProcPipelineBackwardReferenceFrames(PDDI_VP_CONTEXT pVpCtx, VADriverContextP pVaDrvCtx, PVPHAL_SURFACE pVpHalSrcSurf, VAProcPipelineParameterBuffer* pPipelineParam);

/////////////////////////////////////////////////////////////////////////////
//! \purpose Get vp context form context ID
//! \params
//! [in]  VADriverContextP
//! [in]  VAContextID
//! \returns PDDI_VP_CONTEXT
/////////////////////////////////////////////////////////////////////////////
PDDI_VP_CONTEXT DdiVp_GetVpContextFromContextID(VADriverContextP ctx, VAContextID vaCtxID)
{
    uint32_t  uiCtxType;
    return (PDDI_VP_CONTEXT)DdiMedia_GetContextFromContextID(ctx, vaCtxID, &uiCtxType);
}

/////////////////////////////////////////////////////////////////////////////
//! \purpose map from media format to vphal format
//! \params
//! [in]  DDI_MEDIA_FORMAT
//! [out] None
//! \returns MOS_FORMAT
/////////////////////////////////////////////////////////////////////////////
MOS_FORMAT VpGetFormatFromMediaFormat(DDI_MEDIA_FORMAT mf)
{
    MOS_FORMAT format = Format_Invalid;

    VP_DDI_FUNCTION_ENTER;

    switch (mf)
    {
    case Media_Format_NV12:
        format = Format_NV12;
        break;
    case Media_Format_NV21:
        format = Format_NV21;
        break;
    case Media_Format_X8R8G8B8:
        format = Format_X8R8G8B8;
        break;
    case Media_Format_X8B8G8R8:
        format = Format_X8B8G8R8;
        break;
    case Media_Format_A8R8G8B8:
        format = Format_A8R8G8B8;
        break;
    case Media_Format_A8B8G8R8:
        format = Format_A8B8G8R8;
        break;
    case Media_Format_R5G6B5:
        format = Format_R5G6B5;
        break;
    case Media_Format_R8G8B8:
        format = Format_R8G8B8;
        break;
    case Media_Format_YUY2 :
        format = Format_YUY2;
        break;
    case Media_Format_UYVY:
        format = Format_UYVY;
        break;
    case Media_Format_YV12 :
        format = Format_YV12;
        break;
    case Media_Format_I420 :
        format = Format_I420;
        break;
    case Media_Format_IYUV :
        format = Format_IYUV;
        break;
    case Media_Format_422H:
        format = Format_422H;
        break;
    case Media_Format_422V:
        format = Format_422V;
        break;
    case Media_Format_400P:
        format = Format_400P;
        break;
    case Media_Format_411P:
        format = Format_411P;
        break;
    case Media_Format_444P:
        format = Format_444P;
        break;
    case Media_Format_IMC3:
        format = Format_IMC3;
        break;
    case Media_Format_P010:
        format = Format_P010;
        break;
    case Media_Format_R10G10B10A2:
        format = Format_R10G10B10A2;
        break;
    case Media_Format_B10G10R10A2:
        format =Format_B10G10R10A2;
        break;
    default:
        VP_DDI_ASSERTMESSAGE("ERROR media format to vphal format.");
        format = Format_Invalid;
        break;
    }

    return format;
}


/////////////////////////////////////////////////////////////////////////////
//! \purpose Get the VP Tile Type from Media Tile Type.
//! \params
//! [in]  mediaTileType : input mediaTileType
//! [out] None
//! \returns Vp tile Type if call succeeds
/////////////////////////////////////////////////////////////////////////////

MOS_TILE_TYPE VpGetTileTypeFromMediaTileType(uint32_t mediaTileType)
{
    MOS_TILE_TYPE tileType;

    VP_DDI_FUNCTION_ENTER;

    switch(mediaTileType)
    {
       case I915_TILING_Y:
           tileType = MOS_TILE_Y;
           break;
       case I915_TILING_X:
           tileType = MOS_TILE_X;
           break;
       case I915_TILING_NONE:
           tileType = MOS_TILE_LINEAR;
           break;
        default:
           tileType = MOS_TILE_LINEAR;
    }

    return tileType;
}

/////////////////////////////////////////////////////////////////////////////
//! \purpose Get the render parameters  from Va Driver Context.
//! \params
//! [in]  pVpCtx : VP context
//! [out] None
//! \returns Pointer of render parameters
/////////////////////////////////////////////////////////////////////////////
PVPHAL_RENDER_PARAMS VpGetRenderParams(PDDI_VP_CONTEXT pVpCtx)
{
    VP_DDI_FUNCTION_ENTER;

    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", nullptr);
    return pVpCtx->pVpHalRenderParams;
}

/////////////////////////////////////////////////////////////////////////////
//! \purpose Destroy VPHAL Driver Reference Params
//! \params
//! [in]  pSurf : VPHAL surface
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_DestroyVpHalSurface(PVPHAL_SURFACE pSurf)
{
    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pSurf, "Null pSurf.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (pSurf->pFwdRef)
    {
        DdiVp_DestroyVpHalSurface(pSurf->pFwdRef);
    }
    if (pSurf->pBwdRef)
    {
        DdiVp_DestroyVpHalSurface(pSurf->pBwdRef);
    }

    MOS_FreeMemAndSetNull(pSurf);

    return VA_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//! \purpose Destroy VPHAL Driver Source Params
//! \params
//! [in]  pVpCtx : VP context
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_DestroySrcParams(PDDI_VP_CONTEXT pVpCtx)
{
    uint32_t uSurfIndex;

    VP_DDI_FUNCTION_ENTER;

    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    uSurfIndex = 0;

    if (nullptr != pVpCtx)
    {
        if (nullptr != pVpCtx->pVpHalRenderParams)
        {
            for (uSurfIndex = 0; uSurfIndex < VPHAL_MAX_SOURCES; uSurfIndex++)
            {
                if (nullptr != pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex])
                {
                    MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pProcampParams);
                    MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pDeinterlaceParams);
                    MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pDenoiseParams);
                    if (pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pIEFParams)
                    {
                        MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pIEFParams->pExtParam);
                        MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pIEFParams);
                    }
                    MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pBlendingParams);
                    MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pLumaKeyParams);
                    MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pColorPipeParams);
                    DdiVp_DestroyVpHalSurface(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]);
                    pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex] = nullptr;
                }
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//! \purpose Destroy VPHAL Driver Target Params
//! \params
//! [in]  pVpCtx : VP context
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_DestroyTargetParams(PDDI_VP_CONTEXT pVpCtx)
{
    PVPHAL_RENDER_PARAMS pParams;
    PVPHAL_SURFACE       pTarget;
    uint32_t             targetIndex;

    VP_DDI_FUNCTION_ENTER;

    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (pVpCtx->pVpHalRenderParams)
    {
        pParams = pVpCtx->pVpHalRenderParams;
        for (targetIndex = 0; targetIndex < VPHAL_MAX_TARGETS; targetIndex++)
        {
            pTarget = pParams->pTarget[targetIndex];
            if (pTarget)
            {
                if (pTarget->OsResource.bo)
                {
                    pTarget->OsResource.bo = nullptr;
                }
                if (pTarget->pProcampParams)
                {
                    MOS_FreeMemAndSetNull(pTarget->pProcampParams);
                }
                if (pTarget->pDeinterlaceParams)
                {
                    MOS_FreeMemAndSetNull(pTarget->pDeinterlaceParams);
                }
                if (pTarget->pDenoiseParams)
                {
                    MOS_FreeMemAndSetNull(pTarget->pDenoiseParams);
                }

                MOS_FreeMemAndSetNull(pParams->pTarget[targetIndex]);
            }
        }
        // reset render target count
        pParams->uDstCount = 0;
    }

    return VA_STATUS_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
//! \purpose Destroy VPHAL Driver Render Params
//! \params
//! [in]  pVpCtx : VP context
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_DestroyRenderParams(PDDI_VP_CONTEXT pVpCtx)
{
    VP_DDI_FUNCTION_ENTER;

    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    DdiVp_DestroySrcParams(pVpCtx);
    DdiVp_DestroyTargetParams(pVpCtx);

    if (nullptr != pVpCtx)
    {
        if (nullptr != pVpCtx->pVpHalRenderParams)
        {
            MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSplitScreenDemoModeParams);
            MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pColorFillParams);
            MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pCompAlpha);
            MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams);
        }
    }

    return VA_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//! \purpose Destroy VPHAL Driver context
//! \params
//! [in]  pVpCtx : VP context
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_DestroyVpHal(PDDI_VP_CONTEXT pVpCtx)
{
    VP_DDI_FUNCTION_ENTER;

    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    DdiVp_DestroyRenderParams(pVpCtx);

    // Destroy VPHAL context
    if (nullptr != pVpCtx)
    {
        pVpCtx->MosDrvCtx.SkuTable.reset();
        pVpCtx->MosDrvCtx.WaTable.reset();
        if (nullptr != pVpCtx->pVpHal)
        {
            MOS_Delete(pVpCtx->pVpHal);
            pVpCtx->pVpHal = nullptr;
        }

#if (_DEBUG || _RELEASE_INTERNAL)
		VpDestoryDumpConfig(pVpCtx);
#endif //(_DEBUG || _RELEASE_INTERNAL)
    }

    return VA_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Map VA Rotation flags to appropriate VPHAL Rotation params
//! \params
//! [in]  pVpHalSrcSurf
//! [in]  rotation_state
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
VpUpdateProcRotateState(PVPHAL_SURFACE pVpHalSrcSurf, uint32_t rotation_state)
{
    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpHalSrcSurf,  "Null pVpHalSrcSurf.",  VA_STATUS_ERROR_INVALID_PARAMETER);

    switch(rotation_state)
    {
        case VA_ROTATION_NONE:
            pVpHalSrcSurf->Rotation = VPHAL_ROTATION_IDENTITY;
            break;
        case VA_ROTATION_90:
            pVpHalSrcSurf->Rotation = VPHAL_ROTATION_90;
            break;
        case VA_ROTATION_180:
            pVpHalSrcSurf->Rotation = VPHAL_ROTATION_180;
            break;
        case VA_ROTATION_270:
            pVpHalSrcSurf->Rotation = VPHAL_ROTATION_270;
            break;
        default:
            VP_DDI_ASSERTMESSAGE("VpUpdateProcRotateState rotation_state = %d is out of range.", rotation_state);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return VA_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Map VA Mirroring flags to appropriate VPHAL Mirroring params
//! \params
//! [in]  pVpHalSrcSurf
//! [in]  mirror_state
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
VpUpdateProcMirrorState(PVPHAL_SURFACE pVpHalSrcSurf, uint32_t mirror_state)
{
    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpHalSrcSurf,  "Null pVpHalSrcSurf.",  VA_STATUS_ERROR_INVALID_PARAMETER);

    if(mirror_state > VA_MIRROR_VERTICAL)
    {
        VP_DDI_ASSERTMESSAGE("VpUpdateProcMirrorState mirror_state = %d is out of range.", mirror_state);
        VP_DDI_ASSERTMESSAGE("VpUpdateProcMirrorState reset mirror_state to VA_MIRROR_NONE.");
        mirror_state = VA_MIRROR_NONE;
    }

    // Rotation must be a valid angle
    switch(pVpHalSrcSurf->Rotation)
    {
        case VPHAL_ROTATION_IDENTITY:
            if(mirror_state == VA_MIRROR_HORIZONTAL)
            {
                pVpHalSrcSurf->Rotation = VPHAL_MIRROR_HORIZONTAL;
            }
            else if(mirror_state == VA_MIRROR_VERTICAL)
            {
                pVpHalSrcSurf->Rotation = VPHAL_MIRROR_VERTICAL;
            }
            break;
        case VPHAL_ROTATION_90:
            if(mirror_state == VA_MIRROR_HORIZONTAL)
            {
                pVpHalSrcSurf->Rotation = VPHAL_ROTATE_90_MIRROR_HORIZONTAL;
            }
            else if(mirror_state == VA_MIRROR_VERTICAL)
            {
                pVpHalSrcSurf->Rotation = VPHAL_ROTATE_90_MIRROR_VERTICAL;
            }
            break;
        case VPHAL_ROTATION_180:
            if(mirror_state == VA_MIRROR_HORIZONTAL)
            {
                pVpHalSrcSurf->Rotation =  VPHAL_MIRROR_VERTICAL;
            }
            else if(mirror_state == VA_MIRROR_VERTICAL)
            {
                pVpHalSrcSurf->Rotation =  VPHAL_MIRROR_HORIZONTAL;
            }
            break;
        case VPHAL_ROTATION_270:
            if(mirror_state == VA_MIRROR_HORIZONTAL)
            {
                pVpHalSrcSurf->Rotation =  VPHAL_ROTATE_90_MIRROR_VERTICAL;
            }
            else if(mirror_state == VA_MIRROR_VERTICAL)
            {
                pVpHalSrcSurf->Rotation =  VPHAL_ROTATE_90_MIRROR_HORIZONTAL;
            }
            break;
        default:
            VP_DDI_ASSERTMESSAGE("VpUpdateProcMirrorState Unexpected Invalid Rotation = %d.", pVpHalSrcSurf->Rotation);
            return VA_STATUS_ERROR_INVALID_PARAMETER;

    }

    return VA_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Map Chroma Sitting flags to appropriate VPHAL chroma sitting params
//! \params
//! [in]  pVpHalSurf
//! [in]  chromasiting_state
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
VpUpdateProcChromaSittingState(PVPHAL_SURFACE pVpHalSurf, uint8_t chromasiting_state)
{
    uint32_t uChromaSitingFlags  = 0;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpHalSurf, "Null pVpHalSurf.", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Chroma siting 
    // The lower 4 bits are still used as chroma-siting flag for output/input_surface_flag
    // Set the vertical chroma siting info at bit 1:0
    uChromaSitingFlags = chromasiting_state & 0x3;

    switch (uChromaSitingFlags)
    {
    case VA_CHROMA_SITING_VERTICAL_TOP:
        pVpHalSurf->ChromaSiting = CHROMA_SITING_VERT_TOP;
        break;
    case VA_CHROMA_SITING_VERTICAL_CENTER:
        pVpHalSurf->ChromaSiting = CHROMA_SITING_VERT_CENTER;
        break;
    case VA_CHROMA_SITING_VERTICAL_BOTTOM:
        pVpHalSurf->ChromaSiting = CHROMA_SITING_VERT_BOTTOM;
        break;
    default:
        pVpHalSurf->ChromaSiting = CHROMA_SITING_NONE;
        break;
    }

    if (pVpHalSurf->ChromaSiting != CHROMA_SITING_NONE)
    {
        // Set the horizontal chroma siting info at bit 3:2
        uChromaSitingFlags = chromasiting_state & 0xc;

        switch (uChromaSitingFlags)
        {
        case VA_CHROMA_SITING_HORIZONTAL_LEFT:
            pVpHalSurf->ChromaSiting = pVpHalSurf->ChromaSiting | CHROMA_SITING_HORZ_LEFT;
            break;
        case VA_CHROMA_SITING_HORIZONTAL_CENTER:
            pVpHalSurf->ChromaSiting = pVpHalSurf->ChromaSiting | CHROMA_SITING_HORZ_CENTER;
            break;
        default:
            pVpHalSurf->ChromaSiting = CHROMA_SITING_NONE;
            break;
        }
    }

    return VA_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Extract VAProcPipelineParameterBuffer params for target surface and set the appropriate VPHAL params
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpCtx : VP context
//! [in]  pPipelineParam : Pipeline parameters from application (VAProcPipelineParameterBuffer)
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////

VAStatus
VpSetRenderTargetParams(
    VADriverContextP                pVaDrvCtx,
    PDDI_VP_CONTEXT                 pVpCtx,
    VAProcPipelineParameterBuffer*  pPipelineParam)
{
    PVPHAL_RENDER_PARAMS    VpHalRenderParams;
    PDDI_MEDIA_CONTEXT      pMediaCtx;
    PDDI_MEDIA_SURFACE      pMediaSrcSurf;
    PVPHAL_SURFACE          pVpHalTgtSurf;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx, "Null pVaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pPipelineParam, "Null pPipelineParam.", VA_STATUS_ERROR_INVALID_BUFFER);

    pMediaCtx     = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    pMediaSrcSurf = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, pPipelineParam->surface);
    DDI_CHK_NULL(pMediaSrcSurf, "Null pMediaSrcSurf.", VA_STATUS_ERROR_INVALID_BUFFER);
    VpHalRenderParams  = pVpCtx->pVpHalRenderParams;
    DDI_CHK_NULL(VpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    DDI_ASSERT(VpHalRenderParams->uDstCount >= 1);
    DDI_CHK_NULL(VpHalRenderParams->pTarget, "Null VpHalRenderParams->pTarget.", VA_STATUS_ERROR_INVALID_BUFFER);
    pVpHalTgtSurf = VpHalRenderParams->pTarget[VpHalRenderParams->uDstCount-1];
    DDI_CHK_NULL(pVpHalTgtSurf, "Null pVpHalTgtSurf.", VA_STATUS_ERROR_INVALID_BUFFER);

    if (pPipelineParam->surface_region != nullptr)
    {
        pVpHalTgtSurf->rcSrc.left   = pPipelineParam->surface_region->x;
        pVpHalTgtSurf->rcSrc.top    = pPipelineParam->surface_region->y;
        pVpHalTgtSurf->rcSrc.right  = pPipelineParam->surface_region->x + pPipelineParam->surface_region->width;
        pVpHalTgtSurf->rcSrc.bottom = pPipelineParam->surface_region->y + pPipelineParam->surface_region->height;

        if (pVpHalTgtSurf->rcSrc.top < 0)
        {
            pVpHalTgtSurf->rcSrc.top = 0;
        }

        if (pVpHalTgtSurf->rcSrc.left < 0)
        {
            pVpHalTgtSurf->rcSrc.left = 0;
        }

        if (pVpHalTgtSurf->rcSrc.right > pMediaSrcSurf->iWidth)
        {
            pVpHalTgtSurf->rcSrc.right = pMediaSrcSurf->iWidth;
        }

        if (pVpHalTgtSurf->rcSrc.bottom > pMediaSrcSurf->iHeight)
        {
            pVpHalTgtSurf->rcSrc.bottom = pMediaSrcSurf->iHeight;
        }
    }
 
    // Set dest rect
    if (pPipelineParam->output_region != nullptr)
    {
        pVpHalTgtSurf->rcDst.left   = pPipelineParam->output_region->x;
        pVpHalTgtSurf->rcDst.top    = pPipelineParam->output_region->y;
        pVpHalTgtSurf->rcDst.right  = pPipelineParam->output_region->x + pPipelineParam->output_region->width;
        pVpHalTgtSurf->rcDst.bottom = pPipelineParam->output_region->y + pPipelineParam->output_region->height;
        if (pVpHalTgtSurf->rcDst.top < 0)
        {
            pVpHalTgtSurf->rcDst.top = 0;
        }

        if (pVpHalTgtSurf->rcDst.left < 0)
        {
            pVpHalTgtSurf->rcDst.left = 0;
        }

        if (pVpHalTgtSurf->rcDst.right > pMediaSrcSurf->iWidth)
        {
            pVpHalTgtSurf->rcDst.right = pMediaSrcSurf->iWidth;
        }

        if (pVpHalTgtSurf->rcDst.bottom > pMediaSrcSurf->iHeight)
        {
            pVpHalTgtSurf->rcDst.bottom = pMediaSrcSurf->iHeight;
        }
    }
#ifdef ANDROID
    VpUpdateProcChromaSittingState(pVpHalTgtSurf, (uint8_t)(pPipelineParam->output_surface_flag & 0xff));
#else
    VpUpdateProcChromaSittingState(pVpHalTgtSurf, pPipelineParam->output_color_properties.chroma_sample_location);
#endif
    return VA_STATUS_SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Extract VAProcPipelineParameterBuffer params and set the appropriate VPHAL params
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpCtx : VP context
//! [in]  pPipelineParam : Pipeline parameters from application (VAProcPipelineParameterBuffer)
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcPipelineParams(
    VADriverContextP                pVaDrvCtx,
    PDDI_VP_CONTEXT                 pVpCtx,
    VAProcPipelineParameterBuffer*  pPipelineParam)
{
    PVPHAL_RENDER_PARAMS        pVpHalRenderParams;
    PDDI_MEDIA_CONTEXT          pMediaCtx;
    PDDI_MEDIA_SURFACE          pMediaSrcSurf;
    PDDI_MEDIA_BUFFER           pFilterBuf;
    void                        *pData;
    uint32_t                    i;
    PVPHAL_SURFACE              pVpHalSrcSurf;
    PVPHAL_SURFACE              pVpHalTgtSurf;
    uint32_t                    uSurfIndex;
    uint32_t                    uScalingflags;
    uint32_t                    uChromaSitingFlags;
    VAStatus                    vaStatus;
    MOS_STATUS                  eStatus;
    DDI_VP_STATE                vpStateFlags;
    PMOS_INTERFACE              pOsInterface;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx, "Null pVaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    pMediaCtx           = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    pMediaSrcSurf       = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, pPipelineParam->surface);
    pVpHalRenderParams  = pVpCtx->pVpHalRenderParams;
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pFilterBuf          = nullptr;
    pData               = nullptr;
    uSurfIndex          = 0;
    pOsInterface        = pVpCtx->pVpHal->GetOsInterface();

    memset(&vpStateFlags,0,sizeof(vpStateFlags));

    DDI_CHK_NULL(pMediaSrcSurf, "Null pMediaSrcSurf.", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(pOsInterface, "Null pOsInterface.", VA_STATUS_ERROR_INVALID_BUFFER);

    // increment surface count
    pVpHalRenderParams->uSrcCount++;

    // check if the surface count exceeded maximum VPHAL surfaces
    DDI_CHK_CONDITION((pVpHalRenderParams->uSrcCount > VPHAL_MAX_SOURCES),
            "Surface count exceeds maximum!", VA_STATUS_ERROR_MAX_NUM_EXCEEDED);

    // update surface uSurfIndex
    uSurfIndex = pVpHalRenderParams->uSrcCount - 1;

    pVpHalSrcSurf = pVpHalRenderParams->pSrc[uSurfIndex];

    pVpHalSrcSurf->Format = VpGetFormatFromMediaFormat(pMediaSrcSurf->format);
    DDI_CHK_CONDITION((Format_Invalid == pVpHalSrcSurf->Format),
            "Invalid surface media format!", VA_STATUS_ERROR_INVALID_PARAMETER);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pVpCtx->pCurVpDumpDDIParam != nullptr && 
        pVpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex] != nullptr)
    {
        MOS_SecureMemcpy(pVpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[uSurfIndex], sizeof(VAProcPipelineParameterBuffer), pPipelineParam, sizeof(VAProcPipelineParameterBuffer));
        pVpCtx->pCurVpDumpDDIParam->SrcFormat[uSurfIndex] = pVpHalSrcSurf->Format;
    }
#endif //(_DEBUG || _RELEASE_INTERNAL)

    // Set stream type using pipeline_flags VA_PROC_PIPELINE_FAST flag
    // Currently we only support 1 primary surface in VP
    if (pPipelineParam->pipeline_flags & VA_PROC_PIPELINE_FAST)
    {
        pVpHalSrcSurf->SurfType = SURF_IN_SUBSTREAM;
    }
    else
    {
        if (pVpCtx->iPriSurfs < VP_MAX_PRIMARY_SURFS)
        {
            pVpHalSrcSurf->SurfType = SURF_IN_PRIMARY;
            pVpCtx->iPriSurfs++;
        }
        else
        {
            pVpHalSrcSurf->SurfType = SURF_IN_SUBSTREAM;
        }
    }

    // Set src rect
    if (pPipelineParam->surface_region != nullptr)
    {
        pVpHalSrcSurf->rcSrc.top    = pPipelineParam->surface_region->y;
        pVpHalSrcSurf->rcSrc.left   = pPipelineParam->surface_region->x;
        pVpHalSrcSurf->rcSrc.right  = pPipelineParam->surface_region->x + pPipelineParam->surface_region->width;
        pVpHalSrcSurf->rcSrc.bottom = pPipelineParam->surface_region->y + pPipelineParam->surface_region->height;

        if (pVpHalSrcSurf->rcSrc.top < 0)
        {
            pVpHalSrcSurf->rcSrc.top = 0;
        }

        if (pVpHalSrcSurf->rcSrc.left < 0)
        {
            pVpHalSrcSurf->rcSrc.left = 0;
        }

        if (pVpHalSrcSurf->rcSrc.right > pMediaSrcSurf->iWidth)
        {
            pVpHalSrcSurf->rcSrc.right = pMediaSrcSurf->iWidth;
        }

        if (pVpHalSrcSurf->rcSrc.bottom > pMediaSrcSurf->iRealHeight)
        {
            pVpHalSrcSurf->rcSrc.bottom = pMediaSrcSurf->iRealHeight;
        }
    }
    else
    {
        // nullptr surface_region implies the whole surface
        pVpHalSrcSurf->rcSrc.top    = 0;
        pVpHalSrcSurf->rcSrc.left   = 0;
        pVpHalSrcSurf->rcSrc.right  = pMediaSrcSurf->iWidth;
        pVpHalSrcSurf->rcSrc.bottom = pMediaSrcSurf->iRealHeight;
    }

    pVpHalTgtSurf = pVpHalRenderParams->pTarget[0];
    DDI_CHK_NULL(pVpHalTgtSurf, "Null pVpHalTgtSurf.", VA_STATUS_ERROR_UNKNOWN);
    // Set dest rect
    if (pPipelineParam->output_region != nullptr)
    {
        pVpHalSrcSurf->rcDst.top    = pPipelineParam->output_region->y;
        pVpHalSrcSurf->rcDst.left   = pPipelineParam->output_region->x;
        pVpHalSrcSurf->rcDst.right  = pPipelineParam->output_region->x + pPipelineParam->output_region->width;
        pVpHalSrcSurf->rcDst.bottom = pPipelineParam->output_region->y + pPipelineParam->output_region->height;
    }
    else
    {        
        pVpHalSrcSurf->rcDst.top    = 0;
        pVpHalSrcSurf->rcDst.left   = 0;
        pVpHalSrcSurf->rcDst.right  = pVpHalTgtSurf->rcDst.right;
        pVpHalSrcSurf->rcDst.bottom = pVpHalTgtSurf->rcDst.bottom;
    }

    //set the frame_id
    pVpHalSrcSurf->FrameID = pMediaSrcSurf->frame_idx;

    //In order to refresh the frame ID in DdiVp_UpdateFilterParamBuffer when run ADI case,
    //we need to set the OS resource here.
    pVpHalSrcSurf->OsResource.bo          = pMediaSrcSurf->bo;

    // csc option
    //---------------------------------------
    // Set color space for src 
    //---------------------------------------
#ifdef ANDROID
    pVpHalSrcSurf->ColorSpace = DdiVp_GetColorSpace(pPipelineParam->surface_color_standard, pPipelineParam->input_surface_flag);
#else
    pVpHalSrcSurf->ColorSpace = DdiVp_GetColorSpace(pPipelineParam->surface_color_standard, pPipelineParam->input_color_properties.color_range);
#endif
    DDI_CHK_CONDITION((CSpace_None == pVpHalSrcSurf->ColorSpace),
            "Invalid surface color standard", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_CONDITION(((CSpace_BT2020 == pVpHalSrcSurf->ColorSpace) && (Format_P010 != pVpHalSrcSurf->Format)),
            "Invalid surface color standard", VA_STATUS_ERROR_INVALID_PARAMETER);
    if(pMediaSrcSurf->format == Media_Format_400P)
    {
        pVpHalSrcSurf->ColorSpace = CSpace_BT601Gray;
    }
	
    if (IS_RGB_FORMAT(pVpHalSrcSurf->Format))
    {        
        pVpHalSrcSurf->ColorSpace = CSpace_sRGB;
    }
    // extended gamut? RGB can't have extended gamut flag
    pVpHalSrcSurf->ExtendedGamut = false;
	
    // set background colorfill option
    pVpHalRenderParams->pColorFillParams->Color     = pPipelineParam->output_background_color;
    pVpHalRenderParams->pColorFillParams->bYCbCr    = false;
    pVpHalRenderParams->pColorFillParams->CSpace    = CSpace_sRGB;

    // Set Demo Mode option
    if (pVpHalRenderParams->bDisableDemoMode == false)
    {
        eStatus = VpHal_DdiSetupSplitScreenDemoMode(
            0,  // No DDI setting on Linux. Set it when Linux DDI supports it
            0,  // No DDI setting on Linux. Set it when Linux DDI supports it
            &pVpHalRenderParams->pSplitScreenDemoModeParams,
            &pVpHalRenderParams->bDisableDemoMode);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            VP_DDI_ASSERTMESSAGE("Failed to setup Split-Screen Demo Mode.");
            MOS_FreeMemAndSetNull(pVpHalRenderParams->pSplitScreenDemoModeParams);
        }
    }

    // Update fwd and bkward ref frames: Required for Advanced processing - will be supported in the future

    pVpHalSrcSurf->uFwdRefCount  = pPipelineParam->num_forward_references;

    vaStatus = DdiVp_UpdateProcPipelineForwardReferenceFrames(pVpCtx, pVaDrvCtx, pVpHalSrcSurf, pPipelineParam);
    DDI_CHK_RET(vaStatus, "Failed to update forward reference frames!");

    pVpHalSrcSurf->uBwdRefCount  = pPipelineParam->num_backward_references;

    vaStatus = DdiVp_UpdateProcPipelineBackwardReferenceFrames(pVpCtx, pVaDrvCtx, pVpHalSrcSurf, pPipelineParam);
    DDI_CHK_RET(vaStatus, "Failed to update backward reference frames!");
	
    // Check if filter values changed,if yes, then reset all filters for this surface

    // intialize the filter parameter
    // Parameters once set in the surface will be keep till video complete
    // so the parameter of rendered surface should be intialized every time
    if (pVpHalRenderParams->bStereoMode)
    {
        pVpHalSrcSurf->Rotation = VPHAL_ROTATION_IDENTITY;
    }
    
    // Progressive or interlaced - Check filter_flags
    // if the deinterlace parameters is not set, manipulate it as a progressive video.
    if (pPipelineParam->filter_flags & VA_TOP_FIELD)
    {
        pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
    }
    else if (pPipelineParam->filter_flags & VA_BOTTOM_FIELD)
    {
        pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;
    }
    else // VA_FRAME_PICTURE
    {
        pVpHalSrcSurf->SampleType = SAMPLE_PROGRESSIVE;

        //if the deinterlace parameters is set, clear it.
        if (pVpHalSrcSurf->pDeinterlaceParams != nullptr)
        {
            MOS_FreeMemAndSetNull(pVpHalSrcSurf->pDeinterlaceParams);
        }
    }

    for (i = 0; i < pPipelineParam->num_filters; i++)
    {
        VABufferID filter = pPipelineParam->filters[i];

        pFilterBuf = DdiMedia_GetBufferFromVABufferID(pMediaCtx, filter);
        DDI_CHK_NULL(pFilterBuf, "Null pFilterBuf!", VA_STATUS_ERROR_INVALID_BUFFER);

        DDI_CHK_CONDITION((VAProcFilterParameterBufferType != pFilterBuf->uiType),
                "Invalid parameter buffer type!", VA_STATUS_ERROR_INVALID_PARAMETER);

        // Map Buffer data to virtual addres space
        DdiMedia_MapBuffer(pVaDrvCtx, filter, &pData);

        VAProcFilterParameterBufferBase* filter_param = (VAProcFilterParameterBufferBase*) pData;

        // HSBC can only be applied to the primary layer
        if (!((filter_param->type == VAProcFilterColorBalance)
              && pVpHalSrcSurf->SurfType != SURF_IN_PRIMARY))
        {
            // Pass the filter type
            vaStatus = DdiVp_UpdateFilterParamBuffer(pVpCtx, uSurfIndex, filter_param->type, pData, pFilterBuf->iNumElements, &vpStateFlags);
            DDI_CHK_RET(vaStatus, "Failed to update parameter buffer!");
        }
    }

	DdiVp_ClearFilterParamBuffer(pVpCtx, uSurfIndex, vpStateFlags);

    // Update the Deinterlace params
    //vaStatus = DdiVp_UpdateProcDeinterlaceParams(pVaDrvCtx, pVpHalSrcSurf, pPipelineParam);
    //DDI_CHK_RET(vaStatus, "Failed to update vphal advance deinterlace!");

    // Scaling algorithm
    uScalingflags                    = pPipelineParam->filter_flags & VA_FILTER_SCALING_MASK;
    pVpHalSrcSurf->ScalingPreference = VPHAL_SCALING_PREFER_SFC;    // default
    switch (uScalingflags)
    {
    case VA_FILTER_SCALING_FAST:	
	    if (pVpHalSrcSurf->SurfType == SURF_IN_PRIMARY)
        {
            pVpHalSrcSurf->ScalingMode       = VPHAL_SCALING_AVS;
            pVpHalSrcSurf->ScalingPreference = VPHAL_SCALING_PREFER_SFC_FOR_VEBOX;
        }
        else
        {
            pVpHalSrcSurf->ScalingMode       = VPHAL_SCALING_BILINEAR;
        }
        break;    
    case VA_FILTER_SCALING_HQ:
        if (pVpHalSrcSurf->SurfType == SURF_IN_PRIMARY)
        {
            pVpHalSrcSurf->ScalingMode       = VPHAL_SCALING_AVS;
            pVpHalSrcSurf->ScalingPreference = VPHAL_SCALING_PREFER_COMP;
        }
        else
        {
            pVpHalSrcSurf->ScalingMode       = VPHAL_SCALING_BILINEAR;
        }
        break;
    case VA_FILTER_SCALING_DEFAULT:
    case VA_FILTER_SCALING_NL_ANAMORPHIC:
    default:
        if (pVpHalSrcSurf->SurfType == SURF_IN_PRIMARY)
        {
            pVpHalSrcSurf->ScalingMode       = VPHAL_SCALING_AVS;
            pVpHalSrcSurf->ScalingPreference = VPHAL_SCALING_PREFER_SFC;
        }
        else
        {
            pVpHalSrcSurf->ScalingMode       = VPHAL_SCALING_BILINEAR;
        }
        break;
    }
	//init interlace scaling flag
    pVpHalSrcSurf->bInterlacedScaling = false;
    // For interlace scaling
    if (pVpHalSrcSurf->pDeinterlaceParams == nullptr)
    {
        if (pPipelineParam->filter_flags & VA_TOP_FIELD)
        {
            pVpHalSrcSurf->ScalingMode = VPHAL_SCALING_AVS; 
            pVpHalSrcSurf->bInterlacedScaling = true;
        }
        else if (pPipelineParam->filter_flags & VA_BOTTOM_FIELD)
        {
            pVpHalSrcSurf->ScalingMode = VPHAL_SCALING_AVS; 
            pVpHalSrcSurf->bInterlacedScaling = true;
        }
    }
    // For weave DI
    if (pVpHalSrcSurf->pDeinterlaceParams == nullptr)
    {
       if ((pPipelineParam->filter_flags & 0x00000004) || ((pPipelineParam->filter_flags & VA_TOP_FIELD) && pVpHalSrcSurf->bFieldWeaving))
       {
           pVpHalSrcSurf->SampleType = SAMPLE_SINGLE_TOP_FIELD;
		   if (pVpHalSrcSurf->pBwdRef != nullptr)
	       {
	           pVpHalSrcSurf->pBwdRef->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;
	       }
		   pVpHalSrcSurf->ScalingMode = VPHAL_SCALING_BILINEAR;
		   pVpHalSrcSurf->bFieldWeaving = true;
	   }
		   
	   if ((pPipelineParam->filter_flags & 0x00000008) || ((pPipelineParam->filter_flags & VA_BOTTOM_FIELD) && pVpHalSrcSurf->bFieldWeaving))
	   { 
		   pVpHalSrcSurf->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;
		   if (pVpHalSrcSurf->pBwdRef != nullptr)
	       {
	           pVpHalSrcSurf->pBwdRef->SampleType = SAMPLE_SINGLE_TOP_FIELD;
	       }
		   pVpHalSrcSurf->ScalingMode = VPHAL_SCALING_BILINEAR;
		   pVpHalSrcSurf->bFieldWeaving = true;
	   }
    }

    // Rotation
    vaStatus = VpUpdateProcRotateState(pVpHalSrcSurf,pPipelineParam->rotation_state);
    DDI_CHK_RET(vaStatus, "Failed to update rotate state!");

    // Mirror
    vaStatus = VpUpdateProcMirrorState(pVpHalSrcSurf,pPipelineParam->mirror_state);
    DDI_CHK_RET(vaStatus, "Failed to update mirror state!");

    // Alpha blending
    // Note: the alpha blending region cannot overlay
    vaStatus = DdiVp_SetProcPipelineBlendingParams(pVpCtx, uSurfIndex, pPipelineParam);
    DDI_CHK_RET(vaStatus, "Failed to update Alpha Blending parameter!");
#ifdef ANDROID
    VpUpdateProcChromaSittingState(pVpHalSrcSurf, (uint8_t)(pPipelineParam->input_surface_flag&0xff));
    VpUpdateProcChromaSittingState(pVpHalTgtSurf, (uint8_t)(pPipelineParam->output_surface_flag&0xff));
#else
    VpUpdateProcChromaSittingState(pVpHalSrcSurf, pPipelineParam->input_color_properties.chroma_sample_location);
    VpUpdateProcChromaSittingState(pVpHalTgtSurf, pPipelineParam->output_color_properties.chroma_sample_location);
#endif
    // update OsResource for src surface
    pVpHalSrcSurf->OsResource.Format      = VpGetFormatFromMediaFormat(pMediaSrcSurf->format);
    pVpHalSrcSurf->OsResource.iWidth      = pMediaSrcSurf->iWidth;
    pVpHalSrcSurf->OsResource.iHeight     = pMediaSrcSurf->iHeight;
    pVpHalSrcSurf->OsResource.iPitch      = pMediaSrcSurf->iPitch;
    pVpHalSrcSurf->OsResource.iCount      = pMediaSrcSurf->iRefCount;
    pVpHalSrcSurf->OsResource.bo          = pMediaSrcSurf->bo;
    pVpHalSrcSurf->OsResource.TileType    = VpGetTileTypeFromMediaTileType(pMediaSrcSurf->TileType);
    pVpHalSrcSurf->OsResource.pGmmResInfo = pMediaSrcSurf->pGmmResourceInfo;

	Mos_Solo_SetOsResource(pMediaSrcSurf->pGmmResourceInfo, &pVpHalSrcSurf->OsResource);

    //Set encryption bit for input surface
    //This setting only used for secure VPP test app which do secure VP and provide secure YUV as input
    //Since APP cannot set encryption flag, it will set input_surface_flag to ask driver add encryption bit to input surface
    if (pOsInterface->osCpInterface->IsHMEnabled() && (pPipelineParam->input_surface_flag & VPHAL_SURFACE_ENCRYPTION_FLAG))
    {
        pOsInterface->osCpInterface->SetResourceEncryption(&(pVpHalSrcSurf->OsResource), true);
    }

    // Update the Render Target params - this needs to be done once when Render Target is passed via BeginPicture
    vaStatus = DdiVp_UpdateVphalTargetSurfColorSpace(pVaDrvCtx, pVpCtx, pPipelineParam);
    DDI_CHK_RET(vaStatus, "Failed to update vphal target surface color space!");

    return VA_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose:    Get resource informaton from target surface, set OS Resource for VPHAL
//! \[in]  pVpCtx : VP context
//! \[in]  pboRt : media surface
//! \[in]  targetIndex : index of target surface in VpHal RenderParams.
//! \[out] None
//! \returns:    Result of operation
//////////////////////////////////////////////////////////////////////////////////////////////
VAStatus VpSetOsResource(
    PDDI_VP_CONTEXT     pVpCtx, 
    PDDI_MEDIA_SURFACE  pboRt, 
    uint32_t            targetIndex)
{
    PMOS_RESOURCE           pOsResource;
    PVPHAL_RENDER_PARAMS    pVpHalRenderParams;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // get vphal render parameters
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    // set render target os resource information according to target surface
    pOsResource              = &pVpHalRenderParams->pTarget[targetIndex]->OsResource;
    DDI_CHK_NULL(pOsResource, "Null  pOsResource.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pOsResource->bo          = pboRt->bo;
    pOsResource->bMapped     = pboRt->bMapped;
    pOsResource->Format      = VpGetFormatFromMediaFormat( pboRt->format );
    pOsResource->iWidth      = pboRt->iWidth;
    pOsResource->iHeight     = pboRt->iHeight;
    pOsResource->iPitch      = pboRt->iPitch;
    pOsResource->iCount      = pboRt->iRefCount;
    pOsResource->TileType    = VpGetTileTypeFromMediaTileType(pboRt->TileType);
    pOsResource->pGmmResInfo = pboRt->pGmmResourceInfo;

    Mos_Solo_SetOsResource(pboRt->pGmmResourceInfo, pOsResource);

    return VA_STATUS_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Helper function for VpAllocateDrvCtxExt to Allocate PDDI_VP_CONTEXT
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpCtx : VP context
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_InitCtx(VADriverContextP pVaDrvCtx, PDDI_VP_CONTEXT pVpCtx)
{
    PVPHAL_RENDER_PARAMS            pVpHalRenderParams = nullptr;
    int32_t                         uSurfIndex;
    PDDI_MEDIA_CONTEXT              pMediaCtx;
	VAStatus                        vaStatus;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx, "Null pVaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize VPHAL
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    pVpCtx->MosDrvCtx.bufmgr                = pMediaCtx->pDrmBufMgr;
    pVpCtx->MosDrvCtx.fd                    = pMediaCtx->fd;
    pVpCtx->MosDrvCtx.iDeviceId             = pMediaCtx->iDeviceId;
    pVpCtx->MosDrvCtx.SkuTable              = pMediaCtx->SkuTable;
    pVpCtx->MosDrvCtx.WaTable               = pMediaCtx->WaTable;
    pVpCtx->MosDrvCtx.gtSystemInfo          = *pMediaCtx->pGtSystemInfo;
    pVpCtx->MosDrvCtx.platform              = pMediaCtx->platform;
	
    pVpCtx->MosDrvCtx.ppMediaMemDecompState = &pMediaCtx->pMediaMemDecompState;
    pVpCtx->MosDrvCtx.pfnMemoryDecompress   = pMediaCtx->pfnMemoryDecompress;
    pVpCtx->MosDrvCtx.pPerfData             = (PERF_DATA*)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));
    if( nullptr == pVpCtx->MosDrvCtx.pPerfData)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    DDI_CHK_RET(DdiVp_InitVpHal(pVpCtx), "Call DdiVp_InitVpHal failed");

    // initialize DDI level cp interface
    pVpCtx->pCpDdiInterface = MOS_New(DdiCpInterface, pVpCtx->MosDrvCtx);
    if (nullptr == pVpCtx->pCpDdiInterface)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto FINISH;
    }

    // allocate vphal render param
    pVpHalRenderParams = (PVPHAL_RENDER_PARAMS)(MOS_AllocAndZeroMemory(sizeof(VPHAL_RENDER_PARAMS)));
    if( nullptr == pVpHalRenderParams)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto FINISH;
    }
    
    // initialize vphal render params
    for (uSurfIndex = 0; uSurfIndex < VPHAL_MAX_SOURCES; uSurfIndex++)
    {
        pVpHalRenderParams->pSrc[uSurfIndex] = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
        if( nullptr == pVpHalRenderParams->pSrc[uSurfIndex])
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto FINISH;
        }
    }

    for (uSurfIndex = 0; uSurfIndex < VPHAL_MAX_TARGETS; uSurfIndex++)
    {
        pVpHalRenderParams->pTarget[uSurfIndex] = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
        if( nullptr == pVpHalRenderParams->pTarget[uSurfIndex])
		{
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto FINISH;
        }
    }

    // background Colorfill
    pVpHalRenderParams->pColorFillParams = (PVPHAL_COLORFILL_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORFILL_PARAMS));
    if( nullptr == pVpHalRenderParams->pColorFillParams)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto FINISH;
    }

    // reset source surface count
    pVpHalRenderParams->uSrcCount = 0;
    pVpCtx->MosDrvCtx.wRevision = 0;
    pVpCtx->iPriSurfs           = 0;

    // Add the render param for calculating alpha value.
    // Because can not pass the alpha calculate flag from iVP,
    // need to hardcode here for both Android and Linux.
    pVpHalRenderParams->bCalculatingAlpha = true;

    // put the render param in vp context
    pVpCtx->pVpHalRenderParams    = pVpHalRenderParams;

#if (_DEBUG || _RELEASE_INTERNAL)
    vaStatus = VpInitDumpConfig(pVpCtx);
	DDI_CHK_RET( vaStatus, "Init Dump Config failed");
#endif //(_DEBUG || _RELEASE_INTERNAL)

    return VA_STATUS_SUCCESS;

FINISH:
	
    if(pVpHalRenderParams)
    {
        for (uSurfIndex = 0; uSurfIndex < VPHAL_MAX_SOURCES; uSurfIndex++)
        {
            MOS_FreeMemory(pVpHalRenderParams->pSrc[uSurfIndex]);
        }
		for (uSurfIndex = 0; uSurfIndex < VPHAL_MAX_TARGETS; uSurfIndex++)
        {
            MOS_FreeMemory(pVpHalRenderParams->pTarget[uSurfIndex]);
        }
        MOS_FreeMemory(pVpHalRenderParams->pColorFillParams);

        MOS_FreeMemory(pVpHalRenderParams);
    }

    if (pVpCtx->pCpDdiInterface)
    {
        MOS_Delete(pVpCtx->pCpDdiInterface);
        pVpCtx->pCpDdiInterface = nullptr;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
	VpDestoryDumpConfig(pVpCtx);
#endif //(_DEBUG || _RELEASE_INTERNAL)

    return vaStatus;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Initialize VPHAL State and VPHAL settings per VP context
//! \params
//! [in]  pVpCtx : VP context
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_InitVpHal(
    PDDI_VP_CONTEXT   pVpCtx
)
{
    VphalState                *pVpHal;
    VphalSettings             VpHalSettings;

    VAStatus                  vaStatus;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    vaStatus            = VA_STATUS_ERROR_UNKNOWN;
    pVpHal              = nullptr;

    // Create VpHal state
    if (pVpCtx->pVpHal == nullptr)
    {
        MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;
        pVpHal = VphalState::VphalStateFactory( nullptr, &(pVpCtx->MosDrvCtx), &eStatus);
        
        if (pVpHal && MOS_FAILED(eStatus))
        {
            MOS_Delete(pVpHal);
            pVpHal = nullptr;
        }
        
        if (!pVpHal)
        {
            VP_DDI_ASSERTMESSAGE("Failed to create vphal.");
            MOS_FreeMemAndSetNull(pVpCtx);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        if (nullptr != pVpHal)
        {
            VpHalSettings.maxPhases                = VP_SETTING_MAX_PHASES;
            VpHalSettings.mediaStates              = VP_SETTING_MEDIA_STATES;
            VpHalSettings.sameSampleThreshold      = VP_SETTING_SAME_SAMPLE_THRESHOLD;
            VpHalSettings.disableDnDi              = false;

            // Allocate resources (state heaps, resources, KDLL)
            if (MOS_FAILED(pVpHal->Allocate(&VpHalSettings)))
            {
                VP_DDI_ASSERTMESSAGE("Failed to allocate resources for vphal.");
                MOS_Delete(pVpHal);
                pVpHal = nullptr;
                return vaStatus;
            }
        }

        pVpCtx->pVpHal  = pVpHal;
    }

    return VA_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
////! \purpose Convert VAProcColorStandardType to VPHAL_CSPACE
////! \params
////! [in]  color_standard : VA color standard VAProcColorStandardType
////! [in]  flag : input/output surface flag for full/reduced color range
////! [out] None
////! \returns appropriate VPHAL_CSPACE if call succeeds
///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ANDROID
VPHAL_CSPACE
DdiVp_GetColorSpace(VAProcColorStandardType ColorStandard, uint32_t flag)
#else
VPHAL_CSPACE
DdiVp_GetColorSpace(VAProcColorStandardType ColorStandard, uint8_t color_range)
#endif
{
    VPHAL_CSPACE ColorSpace;

    VP_DDI_FUNCTION_ENTER;

    // Convert VAProcColorStandardType to VPHAL_CSPACE

    ColorSpace = CSpace_sRGB;

    switch (ColorStandard)
    {
        case VAProcColorStandardBT709:
#ifdef ANDROID
            if (flag & VA_SOURCE_RANGE_FULL)
#else
            if (color_range == VA_SOURCE_RANGE_FULL)
#endif
            {
                ColorSpace = CSpace_BT709_FullRange;
            }
            else
            {
                ColorSpace = CSpace_BT709;
            }
            break;
        case VAProcColorStandardBT601:
#ifdef ANDROID
            if (flag & VA_SOURCE_RANGE_FULL)
#else
            if (color_range == VA_SOURCE_RANGE_FULL)
#endif
            {
                ColorSpace = CSpace_BT601_FullRange;
            }
            else
            {
                ColorSpace = CSpace_BT601;
            }
            break;
        case VAProcColorStandardBT470M:
        case VAProcColorStandardBT470BG:
        case VAProcColorStandardSMPTE170M:
        case VAProcColorStandardSMPTE240M:
        case VAProcColorStandardGenericFilm:
        default:
            break;
    }

    return ColorSpace;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Get ColorSpace from the media format
//! \params
//! [in]  format : media format
//! [out] None
//! \returns appropriate VPHAL_CSPACE if call succeeds, CSpace_None otherwise
/////////////////////////////////////////////////////////////////////////////////////////////
VPHAL_CSPACE DdiVp_GetColorSpaceFromMediaFormat(DDI_MEDIA_FORMAT format)
{
    VPHAL_CSPACE ColorSpace = CSpace_None;

    switch (format)
    {
    case Media_Format_X8R8G8B8:
    case Media_Format_CPU:
    case Media_Format_R5G6B5:
    case Media_Format_R8G8B8:
    case Media_Format_A8R8G8B8:
    case Media_Format_R10G10B10A2:
    case Media_Format_B10G10R10A2:
        ColorSpace = CSpace_sRGB;
        break;
    case Media_Format_YUY2:
    case Media_Format_UYVY:
    case Media_Format_YV12:
    case Media_Format_IYUV:
    case Media_Format_NV12:
    case Media_Format_NV21:
    case Media_Format_422H:
    case Media_Format_422V:
    case Media_Format_P010:
        ColorSpace = CSpace_BT601;
        break;
    default:
        VP_DDI_ASSERTMESSAGE("Get ColorSpace from media format: unknown format.");
        ColorSpace = CSpace_None;
        break;
    }

    return ColorSpace;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Extract Render Target params from VAProcPipelineParameterBuffer and set the appropriate VPHAL params for RT
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpCtx : VP context
//! [in]  pPipelineParam : VAProcPipelineParameterBuffer Pipeline paramseter from application
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_UpdateVphalTargetSurfColorSpace(
    VADriverContextP                pVaDrvCtx,
    PDDI_VP_CONTEXT                 pVpCtx,
    VAProcPipelineParameterBuffer*  pPipelineParam)
{
    PVPHAL_RENDER_PARAMS      pVpHalRenderParams;
    PVPHAL_SURFACE            pVpHalTgtSurf;
    DDI_UNUSED(pVaDrvCtx);
    
    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams,
                    "Null pVpHalRenderParams.",
                    VA_STATUS_ERROR_INVALID_PARAMETER);
    pVpHalTgtSurf      = pVpHalRenderParams->pTarget[0];
    DDI_CHK_NULL(pVpHalTgtSurf,
                    "Null pVpHalTgtSurf.",
                    VA_STATUS_ERROR_INVALID_SURFACE);

    // update target surface color standard
#ifdef ANDROID
    pVpHalTgtSurf->ColorSpace = DdiVp_GetColorSpace(pPipelineParam->output_color_standard, pPipelineParam->output_surface_flag);
#else
    pVpHalTgtSurf->ColorSpace = DdiVp_GetColorSpace(pPipelineParam->output_color_standard, pPipelineParam->output_color_properties.color_range);
#endif
    DDI_CHK_CONDITION((CSpace_None == pVpHalTgtSurf->ColorSpace),
            "Invalid output color standard", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_CONDITION((CSpace_BT2020 == pVpHalTgtSurf->ColorSpace),
            "Invalid output color standard", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (IS_RGB_FORMAT(pVpHalTgtSurf->Format))
    {        
        pVpHalTgtSurf->ColorSpace = CSpace_sRGB;
    }

    // extended gamut?
    pVpHalRenderParams->pTarget[0]->ExtendedGamut = false;

    return VA_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Setup the appropriate filter params for VPHAL input surface based on Filter type
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  FilterType : Filter type
//! [in]  pData : Buffer data
//! [in]  uElementNum : number of elements in the buffer(FilterParameter)
//! [in]  vpStateFlags : filter enable status
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
//////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_UpdateFilterParamBuffer(
        PDDI_VP_CONTEXT     pVpCtx,
        uint32_t            uSurfIndex,
        int32_t             FilterType,
        void                *pData,
        uint32_t            uElementNum,
        DDI_VP_STATE*       vpStateFlags)
{
    VAStatus vaStatus;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    vaStatus = VA_STATUS_SUCCESS;

    switch (FilterType){
        case VAProcFilterDeinterlacing:
		    vpStateFlags->bDeinterlaceEnable = true;
            vaStatus = DdiVp_SetProcFilterDinterlaceParams(
                                            pVpCtx,
                                            uSurfIndex,
                                            (VAProcFilterParameterBufferDeinterlacing*) pData);
            break;
        case VAProcFilterNoiseReduction:
			vpStateFlags->bDenoiseEnable = true;
            vaStatus = DdiVp_SetProcFilterDenoiseParams(
                                            pVpCtx,
                                            uSurfIndex,
                                            (VAProcFilterParameterBuffer*) pData);
            break;
        case VAProcFilterSharpening:
		    vpStateFlags->bIEFEnable = true;
            vaStatus = DdiVp_SetProcFilterSharpnessParams(
                                            pVpCtx,
                                            uSurfIndex,
                                            (VAProcFilterParameterBuffer*) pData);
            break;
        case VAProcFilterColorBalance:
			vpStateFlags->bProcampEnable = true;
            vaStatus = DdiVp_SetProcFilterColorBalanceParams(
                                            pVpCtx,
                                            uSurfIndex,
                                            (VAProcFilterParameterBufferColorBalance*) pData,
                                            uElementNum);
            break;
        case VAProcFilterSkinToneEnhancement:
            vaStatus = DdiVp_SetProcFilterSkinToneEnhancementParams(
                                            pVpCtx,
                                            uSurfIndex,
                                            (VAProcFilterParameterBuffer*) pData);
            break;
        case VAProcFilterTotalColorCorrection:
            vaStatus = DdiVp_SetProcFilterTotalColorCorrectionParams(
                                            pVpCtx,
                                            uSurfIndex,
                                            (VAProcFilterParameterBufferTotalColorCorrection*) pData,
                                            uElementNum);
            break;
        case VAProcFilterNone:
            vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
            break;
        default:
            VP_DDI_ASSERTMESSAGE("VAProcFilterType is unknown.");
            vaStatus = VA_STATUS_ERROR_UNSUPPORTED_FILTER;
            break;
    }// switch (type)

    return vaStatus;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose clear filter params which is disabled for VPHAL input surface 
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  vpStateFlags : filter enable status
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
//////////////////////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_ClearFilterParamBuffer(
	     PDDI_VP_CONTEXT     pVpCtx,
         uint32_t            uSurfIndex,
         DDI_VP_STATE        vpStateFlags)
{
     if(!vpStateFlags.bProcampEnable)
     {
         MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pProcampParams);
     }
     if(!vpStateFlags.bDeinterlaceEnable)
     {                 
         MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pDeinterlaceParams);
     }                  
     if(!vpStateFlags.bDenoiseEnable)
     { 
         MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pDenoiseParams);
     }
     if(!vpStateFlags.bIEFEnable)
     {
         if (pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pIEFParams)
         {
            MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pIEFParams->pExtParam);
            MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pIEFParams);
         }
     }

     return VA_STATUS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
//! \purpose Set DI filter params for input VPHAL surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  pDiParamBuff : Pointer to DI param buffer data
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcFilterDinterlaceParams(
    PDDI_VP_CONTEXT                             pVpCtx,
    uint32_t                                    uSurfIndex,
    VAProcFilterParameterBufferDeinterlacing*   pDiParamBuff)
{
    PVPHAL_RENDER_PARAMS      pVpHalRenderParams;
    PVPHAL_SURFACE            pTarget;
    PVPHAL_SURFACE            pSrc;
    VPHAL_DI_MODE             DIMode;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pDiParamBuff,
                    "Null pDiParamBuff.",
                    VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams,
                    "Null pVpHalRenderParams.",
                    VA_STATUS_ERROR_INVALID_PARAMETER);

    pSrc    = pVpHalRenderParams->pSrc[uSurfIndex];
    DDI_CHK_NULL(pSrc, "Null pSrc.", VA_STATUS_ERROR_INVALID_SURFACE);
    pTarget = pVpHalRenderParams->pTarget[0];
    DDI_CHK_NULL(pTarget, "Null pTarget.", VA_STATUS_ERROR_INVALID_SURFACE);

    switch (pDiParamBuff->algorithm)
    {
        case VAProcDeinterlacingBob:
            DIMode = DI_MODE_BOB;
            break;
        case VAProcDeinterlacingMotionAdaptive:
            DIMode = DI_MODE_ADI;
            break;
        case VAProcDeinterlacingWeave:
			pSrc->bFieldWeaving = true;;
			return VA_STATUS_SUCCESS;
		case VAProcDeinterlacingNone:
        case VAProcDeinterlacingMotionCompensated:
        default:
            VP_DDI_ASSERTMESSAGE("Deinterlacing type is unsupported.");
            return VA_STATUS_ERROR_UNIMPLEMENTED;
    }// switch (pDiParamBuff->algorithm)

    if (nullptr == pSrc->pDeinterlaceParams)
    {
        pSrc->pDeinterlaceParams    = (PVPHAL_DI_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_DI_PARAMS));
        DDI_CHK_NULL(pSrc->pDeinterlaceParams, "pSrc->pDeinterlaceParams is NULL", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    if (nullptr == pTarget->pDeinterlaceParams)
    {
        pTarget->pDeinterlaceParams = (PVPHAL_DI_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_DI_PARAMS));
        DDI_CHK_NULL(pTarget->pDeinterlaceParams, "pTarget->pDeinterlaceParams is NULL", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }
	//application detect scene change and then pass parameter to driver.
    if (pDiParamBuff->flags & VA_DEINTERLACING_SCD_ENABLE)
    {
        DIMode = DI_MODE_BOB;
    }
    pSrc->pDeinterlaceParams->DIMode = DIMode;

    pSrc->pDeinterlaceParams->bSingleField = (pDiParamBuff->flags & VA_DEINTERLACING_ONE_FIELD) ? true : false;

    pSrc->pDeinterlaceParams->bEnableFMD   = (pDiParamBuff->flags & VA_DEINTERLACING_FMD_ENABLE) ? true : false;

    //update sample type
    if (pDiParamBuff->flags & VA_DEINTERLACING_BOTTOM_FIELD_FIRST)
    {
        if (pDiParamBuff->flags & VA_DEINTERLACING_BOTTOM_FIELD)
        {
            pSrc->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
        }
        else
        {
            pSrc->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD;
        }
    }
    else
    {
        if (pDiParamBuff->flags & VA_DEINTERLACING_BOTTOM_FIELD)
        {
            pSrc->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;
        }
        else
        {
            pSrc->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
        }
    }
    
    if (pSrc->pDeinterlaceParams->DIMode == DI_MODE_ADI)
    {
        //When pBwdRef is not nullptr and uBwdRefCount is nonzero, ADI can use Bwd Ref frame.
        //Otherwise, ADI shouldn't use Bwd Ref frame.
        if (pSrc->uBwdRefCount && pSrc->pBwdRef != nullptr)
        {
            pSrc->uBwdRefCount           = 1;
            
            //When the bo of the current frame's SRC and reference are same with previous frame's,
            //we should set the frame ID same as previous frame's setting.
            if ( pVpCtx->FrameIDTracer.pLastSrcSurfBo == pSrc->OsResource.bo &&
                 pVpCtx->FrameIDTracer.pLastBwdSurfBo == pSrc->pBwdRef->OsResource.bo &&
                 pVpCtx->FrameIDTracer.uiLastSampleType != pSrc->SampleType)
            {
                pSrc->FrameID          = pVpCtx->FrameIDTracer.uiLastSrcSurfFrameID;
                pSrc->pBwdRef->FrameID = pVpCtx->FrameIDTracer.uiLastBwdSurfFrameID;
            }
            //Otherwise, we should update the values of frame ID and FrameID tracer.
            else
            {
                pSrc->pBwdRef->FrameID       = (VP_SETTING_SAME_SAMPLE_THRESHOLD + 1) * pVpCtx->FrameIDTracer.uiFrameIndex;
                pSrc->FrameID                = pSrc->pBwdRef->FrameID + ( VP_SETTING_SAME_SAMPLE_THRESHOLD + 1);

                pVpCtx->FrameIDTracer.pLastSrcSurfBo = pSrc->OsResource.bo;
                pVpCtx->FrameIDTracer.pLastBwdSurfBo = pSrc->pBwdRef->OsResource.bo;

                pVpCtx->FrameIDTracer.uiLastSrcSurfFrameID = pSrc->FrameID;
                pVpCtx->FrameIDTracer.uiLastBwdSurfFrameID = pSrc->pBwdRef->FrameID;

                pVpCtx->FrameIDTracer.uiLastSampleType = pSrc->SampleType;
            }
        }
        else
        {
            //ADI no reference frame driver only care EVEN/ODD
            if (pDiParamBuff->flags & VA_DEINTERLACING_BOTTOM_FIELD)
            {
                pSrc->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
            }
            else
            {
                pSrc->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
            }
        }

        //int32_t overflow process
        pVpCtx->FrameIDTracer.uiFrameIndex = (pVpCtx->FrameIDTracer.uiFrameIndex + 1 == INT_MAX) ? 1 : pVpCtx->FrameIDTracer.uiFrameIndex + 1;
    }
    
    return VA_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! \purpose Set DN filter params for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  pDnParamBuff : Pointer to DN param buffer data
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcFilterDenoiseParams(
    PDDI_VP_CONTEXT                 pVpCtx,
    uint32_t                        uSurfIndex,
    VAProcFilterParameterBuffer*    pDnParamBuff)
{
    PVPHAL_RENDER_PARAMS      pVpHalRenderParams;
    PVPHAL_SURFACE            pSrc;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams,
                    "Null pVpHalRenderParams.",
                     VA_STATUS_ERROR_INVALID_PARAMETER);
    pSrc = pVpHalRenderParams->pSrc[uSurfIndex];
    DDI_CHK_NULL(pSrc, "Null pSrc.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (nullptr == pSrc->pDenoiseParams)
    {
        pSrc->pDenoiseParams = (PVPHAL_DENOISE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_DENOISE_PARAMS));
    }
    DDI_CHK_NULL(pSrc->pDenoiseParams, "pDenoiseParams MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // denoise caps range is from 0 to 64, out of range parameter is treated as an error
    if (pDnParamBuff->value < NOISEREDUCTION_MIN || pDnParamBuff->value > NOISEREDUCTION_MAX)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    pSrc->pDenoiseParams->fDenoiseFactor = pDnParamBuff->value;

    // Luma and chroma denoise should be always enabled when noise reduction is needed
    pSrc->pDenoiseParams->bEnableLuma    = true;
    pSrc->pDenoiseParams->bEnableChroma  = true;
    pSrc->pDenoiseParams->bAutoDetect    = false;
    pSrc->pDenoiseParams->NoiseLevel     = NOISELEVEL_DEFAULT;

    return VA_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Set Sharpness (Image Enhancement Filter, IEF) filter params for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  pDnParamBuff : Pointer to Sharpness param buffer data
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
//////////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcFilterSharpnessParams(
    PDDI_VP_CONTEXT                 pVpCtx,
    uint32_t                        uSurfIndex,
    VAProcFilterParameterBuffer*    pSharpParamBuff)
{
    PVPHAL_RENDER_PARAMS      pVpHalRenderParams;
    PVPHAL_SURFACE            pSrc;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pSharpParamBuff, "Null pSharpParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pSrc = pVpHalRenderParams->pSrc[uSurfIndex];
    DDI_CHK_NULL(pSrc, "Null pSrc.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (nullptr == pSrc->pIEFParams)
    {
        pSrc->pIEFParams    = (PVPHAL_IEF_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_IEF_PARAMS));
        DDI_CHK_NULL(pSrc->pIEFParams, "pIEFParams MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);       
    }

    // out of range parameter is treated as an error
    if (pSharpParamBuff->value < EDGEENHANCEMENT_MIN || pSharpParamBuff->value > EDGEENHANCEMENT_MAX)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // sharpness option
    // setting with hard code in mplayer side.
    // refer to sharpening section of render_picture_vp() defined in vaapi.c.
    // change flag and factor for testing.
    VpHal_DdiInitIEFParams(pSrc->pIEFParams);
    pSrc->bIEF                      = true;
    pSrc->pIEFParams->bEnabled      = true;
    pSrc->pIEFParams->fIEFFactor    = pSharpParamBuff->value;

    return VA_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! \purpose Set Color Balance (procamp) filter params for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  pColorBalanceParamBuff : Pointer to Colorbalance param buffer data
//! [in]  uElementNum : number of elements in the Colorbalance param buffer data
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcFilterColorBalanceParams(
    PDDI_VP_CONTEXT                             pVpCtx,
    uint32_t                                    uSurfIndex,
    VAProcFilterParameterBufferColorBalance*    pColorBalanceParamBuff,
    uint32_t                                    uElementNum)
{
    PVPHAL_RENDER_PARAMS      pVpHalRenderParams;
    PVPHAL_SURFACE            pSrc;
    uint32_t                  i;
    bool                      bProcamp;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pColorBalanceParamBuff,
                    "Null pColorBalanceParamBuff.",
                    VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pSrc = pVpHalRenderParams->pSrc[uSurfIndex];
    DDI_CHK_NULL(pSrc, "Null pSrc.", VA_STATUS_ERROR_INVALID_SURFACE);

    bProcamp = false;
    for (i = 0; i < uElementNum; i++)
    {
        if ((VAProcColorBalanceHue        == pColorBalanceParamBuff[i].attrib) ||
            (VAProcColorBalanceSaturation == pColorBalanceParamBuff[i].attrib) ||
            (VAProcColorBalanceBrightness == pColorBalanceParamBuff[i].attrib) ||
            (VAProcColorBalanceContrast   == pColorBalanceParamBuff[i].attrib))
        {
            bProcamp = true;
            break;
        }
    }

    if (nullptr == pSrc->pProcampParams &&
        true == bProcamp)
    {
        pSrc->pProcampParams  = (PVPHAL_PROCAMP_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_PROCAMP_PARAMS));
        DDI_CHK_NULL(pSrc->pProcampParams, "Source pProcampParams MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    if (nullptr == pVpHalRenderParams->pTarget[0]->pProcampParams)
    {
        pVpHalRenderParams->pTarget[0]->pProcampParams = (PVPHAL_PROCAMP_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_PROCAMP_PARAMS));
        DDI_CHK_NULL(pVpHalRenderParams->pTarget[0]->pProcampParams, "Target pProcampParams MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    // Needed for ACE
    if (nullptr         == pSrc->pColorPipeParams &&
        SURF_IN_PRIMARY == pSrc->SurfType)
    {
        pSrc->pColorPipeParams = (PVPHAL_COLORPIPE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORPIPE_PARAMS));
        DDI_CHK_NULL(pSrc->pColorPipeParams, "pColorPipeParams MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    // set default value
    if (nullptr != pSrc->pProcampParams)
    {
        pSrc->pProcampParams->fHue          = PROCAMP_HUE_DEFAULT;
        pSrc->pProcampParams->fSaturation   = PROCAMP_SATURATION_DEFAULT;
        pSrc->pProcampParams->fBrightness   = PROCAMP_BRIGHTNESS_DEFAULT;
        pSrc->pProcampParams->fContrast     = PROCAMP_CONTRAST_DEFAULT;
    }

    for (i = 0; i < uElementNum; i++)
    {
        switch (pColorBalanceParamBuff[i].attrib)
        {
        case VAProcColorBalanceHue:
            if (pColorBalanceParamBuff[i].value < PROCAMP_HUE_MIN || pColorBalanceParamBuff[i].value > PROCAMP_HUE_MAX)
            {
                VP_DDI_ASSERTMESSAGE("%d: Hue is out of bounds.", i);
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            if (true == bProcamp)
            {
                pSrc->pProcampParams->bEnabled     = true;
                pSrc->pProcampParams->fHue         = pColorBalanceParamBuff[i].value;
            }
            break;
        case VAProcColorBalanceSaturation:
            if (pColorBalanceParamBuff[i].value < PROCAMP_SATURATION_MIN || pColorBalanceParamBuff[i].value > PROCAMP_SATURATION_MAX)
            {
                VP_DDI_ASSERTMESSAGE("%d: Saturation is out of bounds.", i);
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            if (true == bProcamp)
            {
               pSrc->pProcampParams->bEnabled     = true;
               pSrc->pProcampParams->fSaturation  = pColorBalanceParamBuff[i].value;
            }
            break;
        case VAProcColorBalanceBrightness:
            if (pColorBalanceParamBuff[i].value < PROCAMP_BRIGHTNESS_MIN || pColorBalanceParamBuff[i].value > PROCAMP_BRIGHTNESS_MAX)
            {
                VP_DDI_ASSERTMESSAGE("%d: Brightness is out of bounds.", i);
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            if (true == bProcamp)
            {
                pSrc->pProcampParams->bEnabled     = true;
                pSrc->pProcampParams->fBrightness  = pColorBalanceParamBuff[i].value;
            }
            break;
        case VAProcColorBalanceContrast:
            if (pColorBalanceParamBuff[i].value < PROCAMP_CONTRAST_MIN || pColorBalanceParamBuff[i].value > PROCAMP_CONTRAST_MAX)
            {
                VP_DDI_ASSERTMESSAGE("%d: Contrast is out of bounds.", i);
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            if (true == bProcamp)
            {
                pSrc->pProcampParams->bEnabled     = true;
                pSrc->pProcampParams->fContrast    = pColorBalanceParamBuff[i].value;
            }
            break;
        case VAProcColorBalanceAutoContrast:
            if (SURF_IN_PRIMARY == pSrc->SurfType)
            {
                pSrc->pColorPipeParams->bEnableACE    = true;
                pSrc->pColorPipeParams->dwAceLevel    = ACE_LEVEL_DEFAULT;
                pSrc->pColorPipeParams->dwAceStrength = ACE_STRENGTH_DEFAULT;
            }
            break;
        case VAProcColorBalanceAutoSaturation:
        case VAProcColorBalanceAutoBrightness:
            return VA_STATUS_ERROR_UNIMPLEMENTED;
        case VAProcColorBalanceNone:
        case VAProcColorBalanceCount:
        default:
            VP_DDI_ASSERTMESSAGE("pColorBalanceParamBuff[%d].attrib is unknown.", i);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }// switch (attrib)
    }

    return VA_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Set Skin Tone Enhancement (STE) filter params for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  pSTEParamBuff : Pointer to Skin Tone Enhancement param buffer data
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
//////////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcFilterSkinToneEnhancementParams(
    PDDI_VP_CONTEXT              pVpCtx,
    uint32_t                     uSurfIndex,
    VAProcFilterParameterBuffer* pSTEParamBuff)
{
    PVPHAL_RENDER_PARAMS pVpHalRenderParams;
    PVPHAL_SURFACE       pSrc;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pSTEParamBuff, "Null pSTEParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pSrc = pVpHalRenderParams->pSrc[uSurfIndex];
    DDI_CHK_NULL(pSrc, "Null pSrc.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (SURF_IN_PRIMARY == pSrc->SurfType)
    {
        if (nullptr == pSrc->pColorPipeParams)
        {
            pSrc->pColorPipeParams = (PVPHAL_COLORPIPE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORPIPE_PARAMS));
            DDI_CHK_NULL(pSrc->pColorPipeParams, "pColorPipeParams MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        // out of range parameter is treated as an error
        if (pSTEParamBuff->value < STE_MIN || pSTEParamBuff->value > STE_MAX)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        pSrc->pColorPipeParams->bEnableSTE = true;
        pSrc->pColorPipeParams->SteParams.dwSTEFactor = (uint32_t)pSTEParamBuff->value;
    }

    return VA_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Total Color Correction (TCC) filter params for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  pTCCParamBuff : Pointer to Total Color Correction param buffer data
//! [in]  uElementNum : number of elements in the Total Color Correction param buffer data
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
//////////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcFilterTotalColorCorrectionParams(
    PDDI_VP_CONTEXT                                  pVpCtx,
    uint32_t                                         uSurfIndex,
    VAProcFilterParameterBufferTotalColorCorrection* pTCCParamBuff,
    uint32_t                                         uElementNum)
{
    PVPHAL_RENDER_PARAMS pVpHalRenderParams;
    PVPHAL_SURFACE       pSrc;
    uint32_t             i;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pTCCParamBuff, "Null pTCCParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pSrc = pVpHalRenderParams->pSrc[uSurfIndex];
    DDI_CHK_NULL(pSrc, "Null pSrc.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (SURF_IN_PRIMARY == pSrc->SurfType)
    {
        if (nullptr == pSrc->pColorPipeParams)
        {
            pSrc->pColorPipeParams = (PVPHAL_COLORPIPE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORPIPE_PARAMS));
            DDI_CHK_NULL(pSrc->pColorPipeParams, "pColorPipeParams MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        // set default values
        pSrc->pColorPipeParams->TccParams.Red     = (uint8_t)TCC_DEFAULT;
        pSrc->pColorPipeParams->TccParams.Green   = (uint8_t)TCC_DEFAULT;
        pSrc->pColorPipeParams->TccParams.Blue    = (uint8_t)TCC_DEFAULT;
        pSrc->pColorPipeParams->TccParams.Cyan    = (uint8_t)TCC_DEFAULT;
        pSrc->pColorPipeParams->TccParams.Magenta = (uint8_t)TCC_DEFAULT;
        pSrc->pColorPipeParams->TccParams.Yellow  = (uint8_t)TCC_DEFAULT;

        for (i = 0; i < uElementNum; i++)
        {
            if (pTCCParamBuff[i].value < TCC_MIN || pTCCParamBuff[i].value > TCC_MAX)
                return VA_STATUS_ERROR_INVALID_PARAMETER;

            pSrc->pColorPipeParams->bEnableTCC = true;

            switch (pTCCParamBuff[i].attrib)
            {
            case VAProcTotalColorCorrectionRed:
                pSrc->pColorPipeParams->TccParams.Red     = (uint8_t)pTCCParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionGreen:
                pSrc->pColorPipeParams->TccParams.Green   = (uint8_t)pTCCParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionBlue:
                pSrc->pColorPipeParams->TccParams.Blue    = (uint8_t)pTCCParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionCyan:
                pSrc->pColorPipeParams->TccParams.Cyan    = (uint8_t)pTCCParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionMagenta:
                pSrc->pColorPipeParams->TccParams.Magenta = (uint8_t)pTCCParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionYellow:
                pSrc->pColorPipeParams->TccParams.Yellow  = (uint8_t)pTCCParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionNone:
            case VAProcTotalColorCorrectionCount:
            default:
                VP_DDI_ASSERTMESSAGE("pTCCParamBuff[%d].attrib is unknown.", i);
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Create Media Buffer
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpCtx : VP context
//! [in]  vaBufType : VA buffer type
//! [in]  uiSize : element size
//! [in]  uiNumElements : number of elements
//! [in]  pDataClient : client data
//! [inout] pVaBufID : buffer ID
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_CreateBuffer(
    VADriverContextP           pVaDrvCtx,
    void                       *pVpCtx,
    VABufferType               vaBufType,
    uint32_t                   uiSize,
    uint32_t                   uiNumElements,
    void                       *pDataClient,
    VABufferID*                pVaBufID
)
{
    PDDI_MEDIA_CONTEXT             pMediaCtx;
    PDDI_MEDIA_BUFFER              pBuf;
    VAStatus                       vaStatus;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT pBufferHeapElement;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVaDrvCtx,
                    "Null pVaDrvCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    vaStatus    = VA_STATUS_SUCCESS;
    *pVaBufID   = VA_INVALID_ID;

    pMediaCtx   = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx,
                    "Null pMediaCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    // only for VAProcFilterParameterBufferType and VAProcPipelineParameterBufferType
    if (vaBufType != VAProcFilterParameterBufferType
        && vaBufType != VAProcPipelineParameterBufferType
        && (DdiCpInterface::CheckSupportedBufferForVp(vaBufType) != true)
        )
    {
        VP_DDI_ASSERTMESSAGE("Unsupported Va Buffer Type.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // allocate new buf and init
    pBuf               = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    DDI_CHK_NULL(pBuf, "Null pBuf.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    pBuf->pMediaCtx    = pMediaCtx;
    pBuf->iSize        = uiSize * uiNumElements;
    pBuf->iNumElements = uiNumElements;
    pBuf->uiType       = vaBufType;
    pBuf->format       = Media_Format_Buffer;
    pBuf->uiOffset     = 0;
    pBuf->pData        = (uint8_t*)MOS_AllocAndZeroMemory(uiSize * uiNumElements);
    if (nullptr == pBuf->pData)
    {
        MOS_FreeMemAndSetNull(pBuf);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    pBuf->format       = Media_Format_CPU;

    pBufferHeapElement  = DdiMediaUtil_AllocPMediaBufferFromHeap(pMediaCtx->pBufferHeap);
    if (nullptr == pBufferHeapElement)
    {
        MOS_FreeMemAndSetNull(pBuf->pData);
        MOS_FreeMemAndSetNull(pBuf);
        VP_DDI_ASSERTMESSAGE("Invalid buffer index.");
        return VA_STATUS_ERROR_INVALID_BUFFER;
    }
    pBufferHeapElement->pBuffer      = pBuf;
    pBufferHeapElement->pCtx         = (void *)pVpCtx;
    pBufferHeapElement->uiCtxType    = DDI_MEDIA_CONTEXT_TYPE_VP;
    *pVaBufID                        = pBufferHeapElement->uiVaBufferID;
    pMediaCtx->uiNumBufs++;

    // if there is data from client, then dont need to copy data from client
    if (pDataClient) 
    {
        // copy client data to new buf
        eStatus = MOS_SecureMemcpy(pBuf->pData, uiSize * uiNumElements, pDataClient, uiSize * uiNumElements);
        DDI_CHK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "DDI: Failed to copy client data!", VA_STATUS_ERROR_MAX_NUM_EXCEEDED);
    } 
    else 
    {
        // do nothing if there is no data from client
        vaStatus = VA_STATUS_SUCCESS;
    }

    return vaStatus;
}

/////////////////////////////////////////////////////////////////////////////
//! \purpose Convert source Surface to dest Surface
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  srcSurface : source surface
//! [in]  srcx : x corinates of source surface
//! [in]  srcy : y corinates of source surface
//! [in]  srcw : width of source surface
//! [in]  srch : height of source surface
//! [in]  destSurface : destination surface
//! [in]  destx : x corinates of destination surface
//! [in]  desty : y corinates of destination surface
//! [in]  destw : width of destination surface
//! [in]  desth : height of destination surface
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
///////////////////////////////////////////////////////////////////////////// 
VAStatus DdiVp_ConvertSurface(
    VADriverContextP     pVaDrvCtx,
    DDI_MEDIA_SURFACE    *srcSurface,
    int16_t              srcx,
    int16_t              srcy,
    uint16_t             srcw,
    uint16_t             srch,
    DDI_MEDIA_SURFACE    *dstSurface,
    int16_t              destx,
    int16_t              desty,
    uint16_t             destw,
    uint16_t             desth
)
{
    VAStatus                    vaStatus;
    PVPHAL_SURFACE              pSurface;
    PVPHAL_SURFACE              pTarget;
    PVPHAL_RENDER_PARAMS        pRenderParams;
    VPHAL_DENOISE_PARAMS        DenoiseParams;
    DDI_VP_CONTEXT              vpContext;
    MOS_STATUS                  eStatus;
    RECT                        Rect;
    RECT                        DstRect;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx, "Null pVaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(srcSurface,
                    "Null srcSurface.",
                    VA_STATUS_ERROR_INVALID_SURFACE);

    vaStatus = VA_STATUS_SUCCESS;
    eStatus  = MOS_STATUS_INVALID_PARAMETER;

    memset(&vpContext, 0, sizeof(vpContext));
    DDI_CHK_NULL(pVaDrvCtx, "Null pVaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    vaStatus = DdiVp_InitCtx(pVaDrvCtx, &vpContext);
    DDI_CHK_RET(vaStatus, "Failed to initialize vp Context.");

    pRenderParams = vpContext.pVpHalRenderParams;
    DDI_CHK_NULL(pRenderParams,
                    "Null pRenderParams.",
                    VA_STATUS_ERROR_INVALID_PARAMETER);
    pSurface      =  pRenderParams->pSrc[0];
    DDI_CHK_NULL(pSurface, "Null pSurface.", VA_STATUS_ERROR_INVALID_SURFACE);
    pTarget       =  pRenderParams->pTarget[0];
    DDI_CHK_NULL(pTarget, "Null pTarget.", VA_STATUS_ERROR_INVALID_SURFACE);

    // Source Surface Information
    pSurface->Format               = VpGetFormatFromMediaFormat (srcSurface->format);
    pSurface->SurfType             = SURF_IN_PRIMARY;       // Surface type (context)
    pSurface->SampleType           = SAMPLE_PROGRESSIVE;
    pSurface->ScalingMode          = VPHAL_SCALING_AVS;

    vaStatus = DdiMediaUtil_FillPositionToRect(&Rect,    srcx,  srcy,  srcw,  srch);
    if(vaStatus != VA_STATUS_SUCCESS) goto FINISH;
    vaStatus = DdiMediaUtil_FillPositionToRect(&DstRect, destx, desty, destw, desth);
    if(vaStatus != VA_STATUS_SUCCESS) goto FINISH;

    pSurface->OsResource.Format      = VpGetFormatFromMediaFormat(srcSurface->format);
    pSurface->OsResource.iWidth      = srcSurface->iWidth;
    pSurface->OsResource.iHeight     = srcSurface->iHeight;
    pSurface->OsResource.iPitch      = srcSurface->iPitch;
    pSurface->OsResource.iCount      = 0;
    pSurface->OsResource.TileType    = VpGetTileTypeFromMediaTileType(srcSurface->TileType);
    pSurface->OsResource.bMapped     = srcSurface->bMapped;
    pSurface->OsResource.bo          = srcSurface->bo;
    pSurface->OsResource.pGmmResInfo = srcSurface->pGmmResourceInfo;

	Mos_Solo_SetOsResource(srcSurface->pGmmResourceInfo, &pSurface->OsResource);

    pSurface->ColorSpace            = DdiVp_GetColorSpaceFromMediaFormat(srcSurface->format);
    pSurface->ExtendedGamut         = false;
    pSurface->rcSrc                 = Rect;
    pSurface->rcDst                 = DstRect;

    // Setup render target surface
    pTarget->Format                 = VpGetFormatFromMediaFormat(dstSurface->format);
    pTarget->SurfType               = SURF_IN_PRIMARY; //?

    vaStatus = DdiMediaUtil_FillPositionToRect(&Rect,    destx, desty, destw, desth);
    if(vaStatus != VA_STATUS_SUCCESS) goto FINISH;
    vaStatus = DdiMediaUtil_FillPositionToRect(&DstRect, destx, desty, destw, desth);
    if(vaStatus != VA_STATUS_SUCCESS) goto FINISH;

    pTarget->OsResource.Format      = VpGetFormatFromMediaFormat(dstSurface->format);
    pTarget->OsResource.iWidth      = dstSurface->iWidth;
    pTarget->OsResource.iHeight     = dstSurface->iHeight;
    pTarget->OsResource.iPitch      = dstSurface->iPitch;
    pTarget->OsResource.iCount      = 0;
    pTarget->OsResource.TileType    = VpGetTileTypeFromMediaTileType(dstSurface->TileType);
    pTarget->OsResource.bMapped     = dstSurface->bMapped;
    pTarget->OsResource.bo          = dstSurface->bo;
    pTarget->OsResource.pGmmResInfo = dstSurface->pGmmResourceInfo;

    Mos_Solo_SetOsResource(dstSurface->pGmmResourceInfo, &pTarget->OsResource);

    pTarget->ColorSpace             = DdiVp_GetColorSpaceFromMediaFormat(dstSurface->format);
    pTarget->ExtendedGamut          = false;
    pTarget->rcSrc                  = Rect;
    pTarget->rcDst                  = DstRect;

    pRenderParams->uSrcCount        = 1;

    eStatus   = vpContext.pVpHal->Render(vpContext.pVpHalRenderParams);
    if (MOS_FAILED(eStatus))
    {
        VP_DDI_ASSERTMESSAGE("Failed to call render function.");
        vaStatus = VA_STATUS_ERROR_OPERATION_FAILED;
    }

    memset(&(pTarget->OsResource), 0, sizeof(pTarget->OsResource));

FINISH:
    vaStatus |= DdiVp_DestroyVpHal(&vpContext);
    return vaStatus;
}

/////////////////////////////////////////////////////////////////////////////
//! \purpose Create a new VP context, and put in pVpCtx array
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  vaConfigID : 
//! [in]  iWidth : 
//! [in]  iHeight : 
//! [in]  iFlag : 
//! [in]  vaSurfIDs : 
//! [in]  iNumSurfs : 
//! [inout] pVaCtxID : VA context ID
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_CreateContext (
    VADriverContextP    pVaDrvCtx,
    VAConfigID          vaConfigID,
    int32_t             iWidth,
    int32_t             iHeight,
    int32_t             iFlag,
    VASurfaceID        *vaSurfIDs,
    int32_t             iNumSurfs,
    VAContextID        *pVaCtxID
)
{
    PDDI_MEDIA_CONTEXT                pMediaCtx;
    VAStatus                          vaStatus;
    PDDI_VP_CONTEXT                   pVpCtx;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT pVaCtxHeapElmt;
    DDI_UNUSED(vaConfigID);
    DDI_UNUSED(iWidth);
    DDI_UNUSED(iHeight);
    DDI_UNUSED(iFlag);
    DDI_UNUSED(vaSurfIDs);
    DDI_UNUSED(iNumSurfs);
    
    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx,
                    "Null pVaDrvCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    pVpCtx    = nullptr;
    *pVaCtxID = VA_INVALID_ID;

    pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx,
                    "Null pMediaCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    // allocate pVpCtx
    pVpCtx = (PDDI_VP_CONTEXT)MOS_AllocAndZeroMemory(sizeof(DDI_VP_CONTEXT));
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // init pVpCtx
    vaStatus = DdiVp_InitCtx(pVaDrvCtx, pVpCtx);
    DDI_CHK_RET(vaStatus, "VA_STATUS_ERROR_OPERATION_FAILED");

    DdiMediaUtil_LockMutex(&pMediaCtx->VpMutex);

    // get Free VP context index
    pVaCtxHeapElmt = DdiMediaUtil_AllocPVAContextFromHeap(pMediaCtx->pVpCtxHeap);
    if (nullptr == pVaCtxHeapElmt) 
    {
        MOS_FreeMemAndSetNull(pVpCtx);
        DdiMediaUtil_UnLockMutex(&pMediaCtx->VpMutex);
        VP_DDI_ASSERTMESSAGE("VP Context number exceeds maximum.");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    // store pVpCtx in pMedia
    pVaCtxHeapElmt->pVaContext    = (void *)pVpCtx;
    *pVaCtxID = (VAContextID)(pVaCtxHeapElmt->uiVaContextID + DDI_MEDIA_VACONTEXTID_OFFSET_VP);

    // increate VP context number
    pMediaCtx->uiNumVPs++;

    DdiMediaUtil_UnLockMutex(&pMediaCtx->VpMutex);

    return VA_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//! \purpose Destroy VP context
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  vaCtxID : VA Context ID 
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_DestroyContext (
    VADriverContextP    pVaDrvCtx,
    VAContextID         vaCtxID
    )
{
    PDDI_MEDIA_CONTEXT       pMediaCtx;
    PDDI_VP_CONTEXT          pVpCtx;
    uint32_t                 uiVpIndex;
    uint32_t                 ctxType;
    VAStatus                 vaStatus;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx,
                    "Null pVaDrvCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx,
                    "Null pMediaCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);
    pVpCtx    = (PDDI_VP_CONTEXT)DdiMedia_GetContextFromContextID(pVaDrvCtx, vaCtxID, &ctxType);
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    MOS_FreeMemory(pVpCtx->MosDrvCtx.pPerfData);
    pVpCtx->MosDrvCtx.pPerfData = nullptr;

    if (pVpCtx->pCpDdiInterface)
    {
        MOS_Delete(pVpCtx->pCpDdiInterface);
        pVpCtx->pCpDdiInterface = nullptr;
    }

    // destroy vphal
    vaStatus  = DdiVp_DestroyVpHal(pVpCtx);

    // Get VP context index
    uiVpIndex = vaCtxID & DDI_MEDIA_MASK_VACONTEXTID;

    // remove from context array
    DdiMediaUtil_LockMutex(&pMediaCtx->VpMutex);
    // destroy vp context
    MOS_FreeMemAndSetNull(pVpCtx);
    DdiMediaUtil_ReleasePVAContextFromHeap(pMediaCtx->pVpCtxHeap, uiVpIndex);

    pMediaCtx->uiNumVPs--;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->VpMutex);

    return vaStatus;
}

////////////////////////////////////////////////////////////////////////////////
//! \purpose Get ready to process a picture to a target surface
//! \params
//! [in]  pVaDrvCtx : VA Driver Context
//! [in]  vaCtxID : VA context ID
//! [in]  vaSurfID : target surface ID
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_BeginPicture(
        VADriverContextP    pVaDrvCtx,
        VAContextID         vaCtxID,
        VASurfaceID         vaSurfID)
{
    PDDI_MEDIA_CONTEXT          pMediaDrvCtx;
    PDDI_VP_CONTEXT             pVpCtx;
    uint32_t                    ctxType;
    VAStatus                    vaStatus;
    PVPHAL_RENDER_PARAMS        pVpHalRenderParams;
    PVPHAL_SURFACE              pVpHalTgtSurf;
    PDDI_MEDIA_SURFACE          pMediaTgtSurf;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx,
                    "Null pVaDrvCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    pMediaDrvCtx         = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaDrvCtx,
                    "Null pMediaDrvCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);
    pVpCtx               = (PDDI_VP_CONTEXT)DdiMedia_GetContextFromContextID (
                                pVaDrvCtx, vaCtxID, &ctxType);
    DDI_CHK_NULL(pVpCtx,
                    "Null pVpCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);
    pVpCtx->TargetSurfID = vaSurfID;
    pVpHalRenderParams   = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams,
                    "Null pVpHalRenderParams.",
                    VA_STATUS_ERROR_INVALID_PARAMETER);

    // uDstCount == 0 means no render target is set yet.
    // uDstCount == 1 means 1 render target has been set already.
    // uDstCount == 2 means 2 render targets have been set already.
    DDI_CHK_LESS(pVpHalRenderParams->uDstCount, VPHAL_MAX_TARGETS, 
        "Too many render targets for VP.",
        VA_STATUS_ERROR_INVALID_PARAMETER);

    pVpHalTgtSurf = pVpHalRenderParams->pTarget[pVpHalRenderParams->uDstCount];
    DDI_CHK_NULL(pVpHalTgtSurf,
                    "Null pVpHalTgtSurf.",
                    VA_STATUS_ERROR_INVALID_SURFACE);
    pMediaTgtSurf        = DdiMedia_GetSurfaceFromVASurfaceID(pMediaDrvCtx, vaSurfID);
    DDI_CHK_NULL(pMediaTgtSurf,
                    "Null pMediaTgtSurf.",
                    VA_STATUS_ERROR_INVALID_SURFACE);
                    
    pMediaTgtSurf->pVpCtx = pVpCtx;                    

    // Setup Target VpHal Surface
    pVpHalTgtSurf->SurfType      = SURF_OUT_RENDERTARGET;
    pVpHalTgtSurf->rcSrc.top     = 0;
    pVpHalTgtSurf->rcSrc.left    = 0;
    pVpHalTgtSurf->rcSrc.right   = pMediaTgtSurf->iWidth;
    pVpHalTgtSurf->rcSrc.bottom  = pMediaTgtSurf->iRealHeight;
    pVpHalTgtSurf->rcDst.top     = 0;
    pVpHalTgtSurf->rcDst.left    = 0;
    pVpHalTgtSurf->rcDst.right   = pMediaTgtSurf->iWidth;
    pVpHalTgtSurf->rcDst.bottom  = pMediaTgtSurf->iRealHeight;
    pVpHalTgtSurf->ExtendedGamut = false;

    DDI_CHK_NULL(pVpCtx->pCpDdiInterface, "Null pVpCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);
    pVpCtx->pCpDdiInterface->ResetCpContext();

    // Set os resource for VPHal render
    vaStatus = VpSetOsResource(pVpCtx, pMediaTgtSurf, pVpHalRenderParams->uDstCount);
    DDI_CHK_RET(vaStatus, "Call VpSetOsResource failed");

    pVpHalTgtSurf->Format = pVpHalTgtSurf->OsResource.Format;

    // increase render target count
    pVpHalRenderParams->uDstCount++;

    // reset source surface count
    pVpHalRenderParams->uSrcCount = 0;
    
    pVpHalRenderParams->bReportStatus    = true;
    pVpHalRenderParams->StatusFeedBackID = vaSurfID;

    return VA_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! \purpose judge whether the PipelineParam buffer is for target or not
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpCtx : VP context
//! [in]  pPipelineParam : Pipeline parameters from application (VAProcPipelineParameterBuffer)
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
bool VpIsRenderTarget(
    VADriverContextP                pVaDrvCtx,
    PDDI_VP_CONTEXT                 pVpCtx,
    VAProcPipelineParameterBuffer*  pPipelineParam)
{
    PVPHAL_RENDER_PARAMS    pVpHalRenderParams;
    PDDI_MEDIA_CONTEXT      pMediaCtx;
    PDDI_MEDIA_SURFACE      pMediaSrcSurf;
    PVPHAL_SURFACE          pVpHalTgtSurf;
    bool                    IsTarget = false;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx, "Null pVaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pPipelineParam, "Null pPipelineParam.", VA_STATUS_ERROR_INVALID_BUFFER);

    pMediaCtx       = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    pMediaSrcSurf   = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, pPipelineParam->surface);
    DDI_CHK_NULL(pMediaSrcSurf, "Null pMediaSrcSurf.", VA_STATUS_ERROR_INVALID_BUFFER);
    pVpHalRenderParams  = pVpCtx->pVpHalRenderParams;
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if(pPipelineParam->pipeline_flags == 0 && pVpHalRenderParams->uDstCount >= 1)
    {
        pVpHalTgtSurf = pVpHalRenderParams->pTarget[pVpHalRenderParams->uDstCount-1];
    
        IsTarget = ((pVpHalTgtSurf->OsResource.bo != nullptr)
               &&(pVpHalTgtSurf->OsResource.bo == pMediaSrcSurf->bo));
    }

    return IsTarget;
}
////////////////////////////////////////////////////////////////////////////////
//! \purpose Send buffers to the server
//! \params
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  vpCtxID : context ID
//! [in]  buffers : array of buffer IDs
//! [in]  num_buffers : number of buffers IDs in the array
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_RenderPicture (
    VADriverContextP    pVaDrvCtx,
    VAContextID         vpCtxID,
    VABufferID*         buffers,
    int32_t             num_buffers
)
{
    PDDI_MEDIA_CONTEXT        pMediaCtx;
    PDDI_VP_CONTEXT           pVpCtx;
    PDDI_MEDIA_BUFFER         pBuf;
    int32_t                   i;
    void                      *pData;
    uint32_t                  ctxType;
    VAStatus                  vaStatus;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx,
                    "Null pVaDrvCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    vaStatus = VA_STATUS_SUCCESS;

    pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_OPERATION_FAILED);

    pVpCtx    = (PDDI_VP_CONTEXT)DdiMedia_GetContextFromContextID (pVaDrvCtx, vpCtxID, &ctxType);
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    //num_buffers check
    DDI_CHK_CONDITION(((num_buffers > VPHAL_MAX_SOURCES) || (num_buffers <= 0)),
                         "num_buffers is Invalid.",
                         VA_STATUS_ERROR_INVALID_PARAMETER);

    for (i = 0; i < num_buffers; i++)
    {
        pBuf = DdiMedia_GetBufferFromVABufferID(pMediaCtx, buffers[i]);
        DDI_CHK_NULL(pBuf, "Null pBuf.", VA_STATUS_ERROR_INVALID_BUFFER);

        DdiMedia_MapBuffer(pVaDrvCtx, buffers[i], &pData);
        DDI_CHK_NULL(pData, "Null pData.", VA_STATUS_ERROR_INVALID_BUFFER);

        switch ((int32_t)pBuf->uiType)
        {
            // VP Buffer Types
            case VAProcPipelineParameterBufferType:
                if(VpIsRenderTarget(pVaDrvCtx, pVpCtx,(VAProcPipelineParameterBuffer*)pData))
                {
                    vaStatus=VpSetRenderTargetParams(pVaDrvCtx, pVpCtx,
                                                      (VAProcPipelineParameterBuffer*)pData);
                }
                else
                {
                    vaStatus = DdiVp_SetProcPipelineParams(pVaDrvCtx, pVpCtx,
                                                      (VAProcPipelineParameterBuffer*)pData);
                    DDI_CHK_RET(vaStatus, "Unable to set pipeline parameters");
                }
                break;
            case VAProcFilterParameterBufferType:
                // User is not supposed to pass this buffer type:Refer va_vpp.h
                VP_DDI_ASSERTMESSAGE("Invalid buffer type.");
                return VA_STATUS_ERROR_INVALID_BUFFER;

            default:
                vaStatus = pVpCtx->pCpDdiInterface->RenderPictureForVp(pVaDrvCtx, vpCtxID, pBuf, pData);
                DDI_CHK_RET(vaStatus,"Unsupported buffer type!");
                break;
        }
        DdiMedia_UnmapBuffer(pVaDrvCtx, buffers[i]);
    }
    return VA_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! \purpose
//!  Make the end of rendering for a picture.
//!  The server should start processing all pending operations for this
//!  surface. This call is non-blocking. The client can start another
//!  Begin/Render/End sequence on a different render target.
//! \params
//! [in]  pVaDrvCtx : VA Driver Context
//! [in]  vaCtxID : VA Context ID
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_EndPicture (
        VADriverContextP    pVaDrvCtx,
        VAContextID         vaCtxID)
{
    PDDI_VP_CONTEXT         pVpCtx;
    uint32_t                uiCtxType;
    VphalState              *pVpHal;
    MOS_STATUS              eStatus;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVaDrvCtx,
                    "Null pVaDrvCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    //get VP Context
    pVpCtx = (PDDI_VP_CONTEXT)DdiMedia_GetContextFromContextID (pVaDrvCtx, vaCtxID, &uiCtxType);
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    //Add component tag for VP
    DDI_CHK_NULL(pVpCtx->pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pVpCtx->pVpHalRenderParams->Component = COMPONENT_LibVA;

    pVpHal  = pVpCtx->pVpHal;
    DDI_CHK_NULL(pVpHal, "Null pVpHal.", VA_STATUS_ERROR_INVALID_PARAMETER);
    eStatus = pVpHal->Render(pVpCtx->pVpHalRenderParams);

#if (_DEBUG || _RELEASE_INTERNAL)    
    VpDumpProcPipelineParams(pVaDrvCtx, pVpCtx);

#if ANDROID
    VpReportFeatureMode(pVpCtx);
#endif

#endif //(_DEBUG || _RELEASE_INTERNAL)

    // Reset primary surface count for next render call
    pVpCtx->iPriSurfs = 0;

    // Reset render target count for next render call
    pVpCtx->pVpHalRenderParams->uDstCount = 0;

    if (MOS_FAILED(eStatus))
    {
        VP_DDI_ASSERTMESSAGE("Failed to call render function.");
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    return VA_STATUS_SUCCESS;

}

////////////////////////////////////////////////////////////////////////////////
//! \purpose Check if the format contains alpha channel
//! \params
//! [in]  pSurface : VpHal Surface
//! [out] None
//! \returns true if the format of surface contains alpha channel
////////////////////////////////////////////////////////////////////////////////
static bool hasAlphaInSurface(PVPHAL_SURFACE pSurface)
{
    if (pSurface != nullptr)
    {
        switch (pSurface->Format)
        {
            case Format_P8:            //P8_UNORM: An 8-bit color index which is used to lookup a 32-bit ARGB value in the texture palette.
            case Format_AI44:
            case Format_IA44:
            case Format_A8R8G8B8:
            case Format_A8B8G8R8:
            case Format_A8P8:
            case Format_AYUV:
                return true;
            default:
                return false;
        }
    }

    VP_DDI_ASSERTMESSAGE("No surface handle passed in.\n");
    return false;
}

////////////////////////////////////////////////////////////////////////////////
//! \purpose Set alpha blending params for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uiSurfIndex : uiSurfIndex to the input surface array
//! [in]  pPipelineParam : Pipeline paramseter from application (VAProcPipelineParameterBuffer)
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcPipelineBlendingParams(
    PDDI_VP_CONTEXT                         pVpCtx,
    uint32_t                                uiSurfIndex,
    VAProcPipelineParameterBuffer*          pPipelineParam)
{
    PVPHAL_RENDER_PARAMS      pVpHalRenderParams;
    PVPHAL_SURFACE            pSrc;
    bool                      bGlobalAlpha;
    bool                      bPreMultAlpha;

    const VABlendState * blend_state = pPipelineParam->blend_state;
    bGlobalAlpha = false;
    bPreMultAlpha = false;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pPipelineParam,
                    "Null pPipelineParam.",
                    VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams,
                    "Null pVpHalRenderParams.",
                    VA_STATUS_ERROR_INVALID_PARAMETER);

    pSrc = pVpHalRenderParams->pSrc[uiSurfIndex];
    DDI_CHK_NULL(pSrc, "Null pSrc.", VA_STATUS_ERROR_INVALID_SURFACE);

    if(nullptr == pVpHalRenderParams->pCompAlpha)
    {
        pVpHalRenderParams->pCompAlpha  = (PVPHAL_ALPHA_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_ALPHA_PARAMS));
        DDI_CHK_NULL(pVpHalRenderParams->pCompAlpha, "Null pCompAlpha.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }
	
    // So far vaapi does not have alpha fill mode API.
    // And for blending support, we use VPHAL_ALPHA_FILL_MODE_NONE by default.
    pVpHalRenderParams->pCompAlpha->fAlpha	  = 1.0f;
    pVpHalRenderParams->pCompAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_NONE;

    // First, no Blending
    if(!blend_state)
    {
        if (pSrc->pBlendingParams)
        {
            pSrc->pBlendingParams->BlendType = BLEND_NONE;
            pSrc->pBlendingParams->fAlpha    = 1.0;
        }
        
        if (pSrc->pLumaKeyParams)
        {
            pSrc->pLumaKeyParams->LumaLow    = 0;
            pSrc->pLumaKeyParams->LumaHigh    = 0;
        }

        return VA_STATUS_SUCCESS;
    }

    // Then, process all blending types

    if(blend_state->flags & VA_BLEND_GLOBAL_ALPHA)
        bGlobalAlpha  = true;

    if(blend_state->flags & VA_BLEND_PREMULTIPLIED_ALPHA)
        bPreMultAlpha = true;

    if(nullptr == pSrc->pBlendingParams)
    {
        pSrc->pBlendingParams  = (PVPHAL_BLENDING_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_BLENDING_PARAMS));
        DDI_CHK_NULL(pSrc->pBlendingParams, "Null pBlendingParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Surfaces contains alpha value, will be devide into premultiplied and non-premultiplied:
    //     For premultiplied surface, will support:
    //         a. per-pixel alpha blending;
    //         b. per-pixel plus per-plane alpha blending;
    //     For non-promultilied surface, will support:
    //         a. per-plane alpha blending;
    //         b. per-pixel alpha blending;
    //         c. NOT support per-pixel plus per-plane alpha blending;
    // Surfaces NOT contains alpha value, will not be devided into premultiplied or non-premultiplied:
    //     Will only support per-plane alpha blending.
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    if(hasAlphaInSurface(pSrc))
    {
        // For premultiplied surface, we just support four blending types until now
        if(bPreMultAlpha && !bGlobalAlpha)            // Do per-pixel alpha blending for premultiplied surface.
        {
            pSrc->pBlendingParams->BlendType = BLEND_PARTIAL;
        }
        else if(bPreMultAlpha && bGlobalAlpha)        // Do per-pixel plus per-plane alpha blending For premultiplied surface.
        {
            if(blend_state->global_alpha < 1.0f)
            {
                pSrc->pBlendingParams->BlendType = BLEND_CONSTANT_PARTIAL;
            }
            else
            {
                // Optimization when per-plane alpha value is 1.0
                pSrc->pBlendingParams->BlendType = BLEND_PARTIAL;
            }
            pSrc->pBlendingParams->fAlpha = blend_state->global_alpha;
        }
        else if(!bPreMultAlpha && bGlobalAlpha)       // Do per-plane alpha blending for non-premultiplied surface, not do per-pixel blending
        {
            pSrc->pBlendingParams->BlendType = BLEND_CONSTANT;
            pSrc->pBlendingParams->fAlpha    = blend_state->global_alpha;
            VP_DDI_ASSERTMESSAGE("BLEND_CONSTANT do not support alpha calculating.\n");
        }
        else if(!bPreMultAlpha && !bGlobalAlpha)      // Do per-pixel alpha blending for non-premultiplied surface
        {
            pSrc->pBlendingParams->BlendType = BLEND_SOURCE;
            VP_DDI_ASSERTMESSAGE("BLEND_SOURCE do not support alpha calculating.\n");
        }

        // Not support per-plane plus per-pixel alpha blending for non-premultiplied surface.
    }
    else
    {
        if(bGlobalAlpha)           // Do per-plane alpha blending for surface which not contain alpha value
        {
            if(blend_state->global_alpha < 1.0f)
            {
                pSrc->pBlendingParams->BlendType = BLEND_CONSTANT;
                pSrc->pBlendingParams->fAlpha    = blend_state->global_alpha;
                VP_DDI_ASSERTMESSAGE("BLEND_CONSTANT do not support alpha calculating.\n");
            }
            else
            {
                pSrc->pBlendingParams->BlendType = BLEND_PARTIAL;
                VP_DDI_NORMALMESSAGE("Because BLEND_CONSTANT do not support alpha calculating, use BLEND_PARTIAL instead.\n");
            }

            // bPreMultAlpha should not be true for surfaces which do not contain alpha value,
            // but inorder to make the driver more usable, choose the most reasonable blending mode, but print out a message.
            if(bPreMultAlpha)
            {
                VP_DDI_NORMALMESSAGE("Should not set VA_BLEND_PREMULTIPLIED_ALPHA for surface which do not contain alpha value.\n");
            }
        }
        else
        {
            pSrc->pBlendingParams->BlendType = BLEND_NONE;
            pSrc->pBlendingParams->fAlpha    = 1.0;

            // bPreMultAlpha should not be true for surfaces which do not contain alpha value,
            // but inorder to make the driver more usable, choose the most reasonable blending mode, but print out a message.
            if(bPreMultAlpha)
            {
                VP_DDI_NORMALMESSAGE("Should not set VA_BLEND_PREMULTIPLIED_ALPHA for surface which do not contain alpha value.\n");
            }
        }
    }

    if(blend_state->flags & VA_BLEND_LUMA_KEY)
    {
        if(nullptr == pSrc->pLumaKeyParams)
        {
            pSrc->pLumaKeyParams   = (PVPHAL_LUMAKEY_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_LUMAKEY_PARAMS));
            DDI_CHK_NULL(pSrc->pLumaKeyParams, "Null pLumaKeyParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }
        pSrc->pLumaKeyParams->LumaLow    = (int16_t)(pPipelineParam->blend_state->min_luma * 255);
        pSrc->pLumaKeyParams->LumaHigh   = (int16_t)(pPipelineParam->blend_state->max_luma * 255);
    }

    return VA_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! \purpose Update the forward reference frames for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpHalSrcSurf : VpHal source surface
//! [in]  pPipelineParam : Pipeline parameter from application (VAProcPipelineParameterBuffer)
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_UpdateProcPipelineForwardReferenceFrames(
    PDDI_VP_CONTEXT                 pVpCtx,
    VADriverContextP                pVaDrvCtx,
    PVPHAL_SURFACE                  pVpHalSrcSurf,
    VAProcPipelineParameterBuffer*  pPipelineParam)
{
    PVPHAL_RENDER_PARAMS pVpHalRenderParams;
    PVPHAL_SURFACE       pSurface;
    int i;

    PDDI_MEDIA_CONTEXT pMediaCtx;

    VP_DDI_FUNCTION_ENTER;

    DDI_CHK_NULL(pVpCtx,         "Null pVpCtx!",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVaDrvCtx,      "Null pVaDrvCtx!",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVpHalSrcSurf,  "Null pVpHalSrcSurf!",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pPipelineParam, "Null pPipelineParam!", VA_STATUS_ERROR_INVALID_PARAMETER);

    // initialize
    pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx!", VA_STATUS_ERROR_INVALID_CONTEXT);

    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams,
                    "Null pVpHalRenderParams!",
                    VA_STATUS_ERROR_INVALID_PARAMETER);

    pSurface = pVpHalSrcSurf;

    if (pPipelineParam->forward_references != nullptr)
    {
        for (i = 0;i < pPipelineParam->num_forward_references; i++)
        {
            PDDI_MEDIA_SURFACE pRefSurfBuffObj = nullptr;
            if(pSurface->pFwdRef == nullptr)
            {
                pSurface->pFwdRef = (PVPHAL_SURFACE) MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
                DDI_CHK_NULL(pSurface->pFwdRef, "Null pSurface->pFwdRef!", VA_STATUS_ERROR_ALLOCATION_FAILED);

                pSurface->pFwdRef->Format        = pVpHalSrcSurf->Format;
                pSurface->pFwdRef->SurfType      = pVpHalSrcSurf->SurfType;
                pSurface->pFwdRef->rcSrc         = pVpHalSrcSurf->rcSrc;
                pSurface->pFwdRef->rcDst         = pVpHalSrcSurf->rcDst;
                pSurface->pFwdRef->ColorSpace    = pVpHalSrcSurf->ColorSpace;
                pSurface->pFwdRef->ExtendedGamut = pVpHalSrcSurf->ExtendedGamut;
                pSurface->pFwdRef->SampleType    = pVpHalSrcSurf->SampleType;
                pSurface->pFwdRef->ScalingMode   = pVpHalSrcSurf->ScalingMode;
                pSurface->pFwdRef->OsResource    = pVpHalSrcSurf->OsResource;
                pSurface->pFwdRef->dwWidth       = pVpHalSrcSurf->dwWidth;
                pSurface->pFwdRef->dwHeight      = pVpHalSrcSurf->dwHeight;
                pSurface->pFwdRef->dwPitch       = pVpHalSrcSurf->dwPitch;
                pSurface->uFwdRefCount           = pPipelineParam->num_forward_references - i;
            }
            pRefSurfBuffObj = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, pPipelineParam->forward_references[i]);
            DDI_CHK_NULL(pRefSurfBuffObj,
                            "Null pRefSurfBuffObj!",
                             VA_STATUS_ERROR_INVALID_SURFACE);

            pSurface->pFwdRef->OsResource.bo          = pRefSurfBuffObj->bo;
            pSurface->pFwdRef->OsResource.Format      = VpGetFormatFromMediaFormat(pRefSurfBuffObj->format);
            pSurface->pFwdRef->OsResource.iWidth      = pRefSurfBuffObj->iWidth;
            pSurface->pFwdRef->OsResource.iHeight     = pRefSurfBuffObj->iHeight;
            pSurface->pFwdRef->OsResource.iPitch      = pRefSurfBuffObj->iPitch;
            pSurface->pFwdRef->OsResource.TileType    = VpGetTileTypeFromMediaTileType(pRefSurfBuffObj->TileType);
            pSurface->pFwdRef->OsResource.pGmmResInfo = pRefSurfBuffObj->pGmmResourceInfo;

		    Mos_Solo_SetOsResource(pRefSurfBuffObj->pGmmResourceInfo, &pSurface->OsResource);

            pSurface->pFwdRef->FrameID                = pRefSurfBuffObj->frame_idx;

            pSurface = pSurface->pFwdRef;
        }
     }

     return VA_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! \purpose Update the backward reference frames for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpHalSrcSurf : VpHal source surface
//! [in]  pPipelineParam : Pipeline parameter from application (VAProcPipelineParameterBuffer)
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_UpdateProcPipelineBackwardReferenceFrames(
    PDDI_VP_CONTEXT                 pVpCtx,
    VADriverContextP                pVaDrvCtx,
    PVPHAL_SURFACE                  pVpHalSrcSurf,
    VAProcPipelineParameterBuffer*  pPipelineParam)
{
    PVPHAL_RENDER_PARAMS pVpHalRenderParams;
    PVPHAL_SURFACE       pSurface;
    int i;

    PDDI_MEDIA_CONTEXT pMediaCtx;

    VP_DDI_FUNCTION_ENTER;

    DDI_CHK_NULL(pVpCtx,         "Null pVpCtx!",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVaDrvCtx,      "Null pVaDrvCtx!",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVpHalSrcSurf,  "Null pVpHalSrcSurf!",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pPipelineParam, "Null pPipelineParam!", VA_STATUS_ERROR_INVALID_PARAMETER);

    // initialize
    pMediaCtx = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx!", VA_STATUS_ERROR_INVALID_CONTEXT);

    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams,
                    "Null pVpHalRenderParams!",
                    VA_STATUS_ERROR_INVALID_PARAMETER);

    pSurface = pVpHalSrcSurf;

    if (pPipelineParam->backward_references != nullptr)
    {
        for (i = 0;i < pPipelineParam->num_backward_references; i++)
        {
            PDDI_MEDIA_SURFACE pRefSurfBuffObj = nullptr;
            if(pSurface->pBwdRef == nullptr)
            {
                pSurface->pBwdRef = (PVPHAL_SURFACE) MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
                DDI_CHK_NULL(pSurface->pBwdRef, "Null pSurface->pBwdRef!", VA_STATUS_ERROR_ALLOCATION_FAILED);

                pSurface->pBwdRef->Format        = pVpHalSrcSurf->Format;
                pSurface->pBwdRef->SurfType      = pVpHalSrcSurf->SurfType;
                pSurface->pBwdRef->rcSrc         = pVpHalSrcSurf->rcSrc;
                pSurface->pBwdRef->rcDst         = pVpHalSrcSurf->rcDst;
                pSurface->pBwdRef->ColorSpace    = pVpHalSrcSurf->ColorSpace;
                pSurface->pBwdRef->ExtendedGamut = pVpHalSrcSurf->ExtendedGamut;
                pSurface->pBwdRef->SampleType    = pVpHalSrcSurf->SampleType;
                pSurface->pBwdRef->ScalingMode   = pVpHalSrcSurf->ScalingMode;
                pSurface->pBwdRef->OsResource    = pVpHalSrcSurf->OsResource;
                pSurface->pBwdRef->dwWidth       = pVpHalSrcSurf->dwWidth;
                pSurface->pBwdRef->dwHeight      = pVpHalSrcSurf->dwHeight;
                pSurface->pBwdRef->dwPitch       = pVpHalSrcSurf->dwPitch;
                pSurface->uBwdRefCount           = pPipelineParam->num_backward_references - i;;
            }
            pRefSurfBuffObj = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, pPipelineParam->backward_references[i]);
            DDI_CHK_NULL(pRefSurfBuffObj,
                            "Null pRefSurfBuffObj!",
                             VA_STATUS_ERROR_INVALID_SURFACE);

            pSurface->pBwdRef->OsResource.bo          = pRefSurfBuffObj->bo;
            pSurface->pBwdRef->OsResource.Format      = VpGetFormatFromMediaFormat(pRefSurfBuffObj->format);
            pSurface->pBwdRef->OsResource.iWidth      = pRefSurfBuffObj->iWidth;
            pSurface->pBwdRef->OsResource.iHeight     = pRefSurfBuffObj->iHeight;
            pSurface->pBwdRef->OsResource.iPitch      = pRefSurfBuffObj->iPitch;
            pSurface->pBwdRef->OsResource.TileType    = VpGetTileTypeFromMediaTileType(pRefSurfBuffObj->TileType);
            pSurface->pBwdRef->OsResource.pGmmResInfo = pRefSurfBuffObj->pGmmResourceInfo;

		    Mos_Solo_SetOsResource(pRefSurfBuffObj->pGmmResourceInfo, &pSurface->OsResource);

            pSurface->pBwdRef->FrameID                = pRefSurfBuffObj->frame_idx;

            pSurface = pSurface->pBwdRef;
        }
     }

     return VA_STATUS_SUCCESS;
}

/** \brief Capabilities specification for the color balance filter. */
static VAProcFilterCapColorBalance VpColorBalCap[] = {
    /** \brief Hue. */
    {VAProcColorBalanceHue,
        { PROCAMP_HUE_MIN, 
          PROCAMP_HUE_MAX, 
          PROCAMP_HUE_DEFAULT, 
          PROCAMP_HUE_STEP }
    },
    /** \brief Saturation. */
    {VAProcColorBalanceSaturation,
        { PROCAMP_SATURATION_MIN, 
          PROCAMP_SATURATION_MAX, 
          PROCAMP_SATURATION_DEFAULT, 
          PROCAMP_SATURATION_STEP }
    },
    /** \brief Brightness. */
    {VAProcColorBalanceBrightness,
        { PROCAMP_BRIGHTNESS_MIN, 
          PROCAMP_BRIGHTNESS_MAX, 
          PROCAMP_BRIGHTNESS_DEFAULT, 
          PROCAMP_BRIGHTNESS_STEP }
    },
    /** \brief Contrast. */
    {VAProcColorBalanceContrast,
        { PROCAMP_CONTRAST_MIN, 
          PROCAMP_CONTRAST_MAX, 
          PROCAMP_CONTRAST_DEFAULT, 
          PROCAMP_CONTRAST_STEP }
    },
    /** \brief Automatically adjusted contrast. */
    {VAProcColorBalanceAutoContrast,
        { 0.0F, 0.0F, 0.0F, 0.0F }
    }
};

////////////////////////////////////////////////////////////////////////////////
//! \purpose Query video processing filter capabilities
//! \params
//! [in]     pVaDrvCtx : VA Driver context
//! [in]     pVaCtxID : VA context ID
//! [in]     type :
//! [inout]  filter_caps :
//! [inout]  num_filter_caps :
//! [out]    None
//! \returns VA_STATUS_SUCCESS if call succeeds
//! THIS
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_QueryVideoProcFilterCaps (
    VADriverContextP    pVaDrvCtx,
    VAContextID         pVaCtxID,
    int32_t             type,
    void                *filter_caps,
    uint32_t            *num_filter_caps
)
{
#define QUERY_CAPS_ATTRIBUTE 1 /* query the actual filter caps attribute in vp module */

    uint32_t uCnt;            /* used for loop                               */
    uint32_t uQueryCapsNum;   /* the filter caps number queried by app layer */
    uint32_t uExistCapsNum;   /* the actual number of filters in vp module   */
    uint32_t uQueryFlag = 0;  /* QUERY_CAPS_ATTRIBUTE: search caps attribute */
    DDI_UNUSED(pVaCtxID);
    
    VP_DDI_FUNCTION_ENTER;

    DDI_CHK_NULL (filter_caps, "Null filter_caps.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL (num_filter_caps, "Null num_filter_caps.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (*num_filter_caps != 0)
    {
        /* return filter caps attribute  */
        uQueryFlag = QUERY_CAPS_ATTRIBUTE;
    }

    uQueryCapsNum = *num_filter_caps;

    switch (type)
    {
        /* Noise reduction filter */
        case VAProcFilterNoiseReduction:
            uExistCapsNum    = 1;
            /* set input filter caps number to the actual number of filters in vp module */
            *num_filter_caps = uExistCapsNum;
            /* set the actual filter caps attribute in vp module */
            if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
            {
                VAProcFilterCap* baseCap     = (VAProcFilterCap*) filter_caps;

                baseCap->range.min_value     = NOISEREDUCTION_MIN;
                baseCap->range.max_value     = NOISEREDUCTION_MAX;
                baseCap->range.default_value = NOISEREDUCTION_DEFAULT;
                baseCap->range.step          = NOISEREDUCTION_STEP;


                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }
            }
            break;

        /* Deinterlacing filter */
        case VAProcFilterDeinterlacing:
            uExistCapsNum  = 2;
            /* set input filter caps number to the actual number of filters in vp module */
            *num_filter_caps = uExistCapsNum;
            /* set the actual filter caps attribute in vp module */
            if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
            {
                VAProcFilterCapDeinterlacing* diCap = (VAProcFilterCapDeinterlacing*) filter_caps;

                diCap[0].type                         = VAProcDeinterlacingBob;
                diCap[1].type                         = VAProcDeinterlacingMotionAdaptive;

                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }
            }
            break;

        /* Sharpening filter. */
        case VAProcFilterSharpening:
            uExistCapsNum  = 1;
            /* set input filter caps number to the actual number of filters in vp module */
            *num_filter_caps = uExistCapsNum;
            /* set the actual filter caps attribute in vp module */
            if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
            {
                VAProcFilterCap* baseCap;

                baseCap                      = (VAProcFilterCap*) filter_caps;
                baseCap->range.min_value     = EDGEENHANCEMENT_MIN;
                baseCap->range.max_value     = EDGEENHANCEMENT_MAX;
                baseCap->range.default_value = EDGEENHANCEMENT_DEFAULT;
                baseCap->range.step          = EDGEENHANCEMENT_STEP;

                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }
            }
            break;

        /* Color balance parameters. */
        case VAProcFilterColorBalance:
            uExistCapsNum = sizeof (VpColorBalCap)/sizeof (VAProcFilterCapColorBalance);
            /* set input filter caps number to the actual number of filters in vp module */
            *num_filter_caps = uExistCapsNum;
            /* set the actual filter caps attribute in vp module */
            if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
            {
                VAProcFilterCapColorBalance* ColorBalCap;

                for (uCnt=0 ; uCnt<uQueryCapsNum ; uCnt++)
                {
                    /* the actual filter caps attribute is not enough */
                    if (uCnt >= uExistCapsNum)
                    {
                        break;
                    }

                    ColorBalCap                         = (VAProcFilterCapColorBalance*) filter_caps+uCnt;
                    ColorBalCap->type                   = VpColorBalCap[uCnt].type;
                    ColorBalCap->range.default_value    = VpColorBalCap[uCnt].range.default_value;
                    ColorBalCap->range.max_value        = VpColorBalCap[uCnt].range.max_value;
                    ColorBalCap->range.min_value        = VpColorBalCap[uCnt].range.min_value;
                    ColorBalCap->range.step             = VpColorBalCap[uCnt].range.step;
                }

                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }
            }
            break;
        case VAProcFilterSkinToneEnhancement:
            *num_filter_caps = 1;
            if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
            {
                VAProcFilterCap* baseCap     = (VAProcFilterCap *)filter_caps;

                baseCap->range.min_value     = STE_MIN;
                baseCap->range.max_value     = STE_MAX;
                baseCap->range.default_value = STE_DEFAULT;
                baseCap->range.step          = STE_STEP;
            }
            break;

        case VAProcFilterTotalColorCorrection:
            uExistCapsNum = 6;
            *num_filter_caps = uExistCapsNum;
            if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
            {
                VAProcFilterCapTotalColorCorrection* TccCap;

                for (uCnt = 0; uCnt < uQueryCapsNum; uCnt++)
                {
                    if (uCnt >= uExistCapsNum)
                        break;

                    TccCap                      = (VAProcFilterCapTotalColorCorrection *)filter_caps + uCnt;
                    TccCap->type                = (VAProcTotalColorCorrectionType)((uint32_t)VAProcTotalColorCorrectionRed + uCnt);
                    TccCap->range.min_value     = TCC_MIN;
                    TccCap->range.max_value     = TCC_MAX;
                    TccCap->range.default_value = TCC_DEFAULT;
                    TccCap->range.step          = TCC_STEP;
                }

                if (uQueryCapsNum < uExistCapsNum)
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            }
            break;
        case VAProcFilterCount:
        case VAProcFilterNone:
            return VA_STATUS_ERROR_INVALID_VALUE;

        default:
            return VA_STATUS_ERROR_UNSUPPORTED_FILTER;

    }

    return VA_STATUS_SUCCESS;
}// DdiVp_QueryVideoProcFilterCaps()
