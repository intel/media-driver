/*
* Copyright (c) 2009-2022, Intel Corporation
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

#include "media_libva_vp_tools.h"
#if (_DEBUG || _RELEASE_INTERNAL)
#if ANDROID
#include "media_libva_vp_tools_android.h"
#endif
#endif // #if (_DEBUG || _RELEASE_INTERNAL)

#define VP_SETTING_MAX_PHASES                           1
#define VP_SETTING_MEDIA_STATES                         32
#define VP_SETTING_SAME_SAMPLE_THRESHOLD                1000

// ITU-T H.265 Table E.3: Colour Primaries
#define COLOUR_PRIMARY_BT2020            9
#define COLOUR_PRIMARY_BT709             1
#define COLOUR_PRIMARY_BT601             5
// ITU-T H.265 Table E.4 Transfer characteristics
#define TRANSFER_CHARACTERISTICS_BT709         1
#define TRANSFER_CHARACTERISTICS_GMAMA2P2      4
#define TRANSFER_CHARACTERISTICS_LINEAR        8
#define TRANSFER_CHARACTERISTICS_BT2020        14
#define TRANSFER_CHARACTERISTICS_ST2084        16

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
VAStatus     DdiVp_InitCtx(VADriverContextP, PDDI_VP_CONTEXT);
VAStatus     DdiVp_UpdateFilterParamBuffer(VADriverContextP, PDDI_VP_CONTEXT, uint32_t, int32_t, void *, uint32_t, DDI_VP_STATE*);
VAStatus     DdiVp_ClearFilterParamBuffer(PDDI_VP_CONTEXT , uint32_t, DDI_VP_STATE);
VAStatus     DdiVp_SetProcFilterDinterlaceParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBufferDeinterlacing*);
VAStatus     DdiVp_SetProcFilterDenoiseParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBuffer*);
VAStatus     DdiVp_SetProcFilterHVSDenoiseParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBufferHVSNoiseReduction*);
VAStatus     DdiVp_SetProcFilterSharpnessParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBuffer*);
VAStatus     DdiVp_SetProcFilterColorBalanceParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBufferColorBalance*, uint32_t );
VAStatus     DdiVp_SetProcFilterSkinToneEnhancementParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBuffer*);
VAStatus     DdiVp_SetProcFilterTotalColorCorrectionParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBufferTotalColorCorrection*, uint32_t);
VAStatus     DdiVp_SetProcFilterHdrTmParams(PDDI_VP_CONTEXT, uint32_t, VAProcFilterParameterBufferHDRToneMapping*);
VAStatus     DdiVp_SetProcPipelineBlendingParams(PDDI_VP_CONTEXT pVpCtx, uint32_t uiSurfIndex, VAProcPipelineParameterBuffer* pPipelineParam);
VAStatus     DdiVp_ConvertSurface (VADriverContextP ctx, DDI_MEDIA_SURFACE  *srcSurface, int16_t srcx,  int16_t srcy, uint16_t srcw,  uint16_t srch,  DDI_MEDIA_SURFACE  *dstSurface,  int16_t destx,  int16_t desty, uint16_t destw, uint16_t desth );
VAStatus     DdiVp_UpdateProcPipelineFutureReferenceFrames(PDDI_VP_CONTEXT pVpCtx, VADriverContextP pVaDrvCtx, PVPHAL_SURFACE pVpHalSrcSurf, VAProcPipelineParameterBuffer* pPipelineParam);
VAStatus     DdiVp_UpdateProcPipelinePastReferenceFrames(PDDI_VP_CONTEXT pVpCtx, VADriverContextP pVaDrvCtx, PVPHAL_SURFACE pVpHalSrcSurf, VAProcPipelineParameterBuffer* pPipelineParam);
VAStatus     DdiVp_UpdateVphalTargetSurfColorSpace(VADriverContextP, PDDI_VP_CONTEXT, VAProcPipelineParameterBuffer*, uint32_t targetIndex);
VAStatus     DdiVp_BeginPictureInt(VADriverContextP pVaDrvCtx, PDDI_VP_CONTEXT pVpCtx, VASurfaceID vaSurfID);
#if VA_CHECK_VERSION(1, 12, 0)
VAStatus     DdiVp_SetProcFilter3DLutParams(VADriverContextP pVaDrvCtx, PDDI_VP_CONTEXT pVpCtx, uint32_t uSurfIndex, VAProcFilterParameterBuffer3DLUT* p3DLutParamBuff);
#endif
#if (VA_MAJOR_VERSION < 1)
VAStatus     DdiVp_GetColorSpace(PVPHAL_SURFACE pVpHalSurf, VAProcColorStandardType colorStandard, uint32_t flag);
#else
VAStatus     DdiVp_GetColorSpace(PVPHAL_SURFACE pVpHalSurf, VAProcColorStandardType colorStandard, VAProcColorProperties colorProperties);
#endif


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

////////////////////////////////////////////////////////////////////////////////
//! \purpose judge whether the pitch size match 16aligned usrptr path require or not
//! \params
//! [in]  pitch: surface pitch size.
//! [in]  format : surface foramt
//! \returns true if matched
//! for YV12 format, if pitch aligned with 128, go legacy path; if aligned with 16/32/64, go 16usrptr path
//! for other formats, legcy path for aligned with 64, 16usrpt path for aligned with 16/32
////////////////////////////////////////////////////////////////////////////////
bool VpIs16UsrPtrPitch(uint32_t pitch, DDI_MEDIA_FORMAT format)
{
    uint32_t PitchAligned = 64;
    bool status = false;

    if (Media_Format_YV12 == format)
    {
        PitchAligned = 128;
    }

    if (!(pitch % 16) && (pitch % PitchAligned))
    {
        status = true;
    }
    else
    {
        status = false;
    }

    VP_DDI_NORMALMESSAGE("[VP] 16Usrptr check, surface pitch is %d, go to %s path.", pitch, status?"16Usrptr":"legacy");

    return status;
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
    case Media_Format_R8G8B8A8:
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
    case Media_Format_P012:
    case Media_Format_P016:
        format = Format_P016;
        break;
    case Media_Format_R10G10B10A2:
    case Media_Format_R10G10B10X2:
        format = Format_R10G10B10A2;
        break;
    case Media_Format_B10G10R10A2:
    case Media_Format_B10G10R10X2:
        format =Format_B10G10R10A2;
        break;
    case Media_Format_RGBP:
        format = Format_RGBP;
        break;
    case Media_Format_BGRP:
        format = Format_BGRP;
        break;
    case Media_Format_Y210:
        format = Format_Y210;
        break;
#if VA_CHECK_VERSION(1, 9, 0)
    case Media_Format_Y212:
#endif
    case Media_Format_Y216:
        format = Format_Y216;
        break;
    case Media_Format_Y410:
        format = Format_Y410;
        break;
#if VA_CHECK_VERSION(1, 9, 0)
    case Media_Format_Y412:
#endif
    case Media_Format_Y416:
        format = Format_Y416;
        break;
    case Media_Format_AYUV:
#if VA_CHECK_VERSION(1, 13, 0)
    case Media_Format_XYUV:
#endif
        format = Format_AYUV;
        break;
    case Media_Format_Y8:
        format = Format_Y8;
        break;
    case Media_Format_Y16S:
        format = Format_Y16S;
        break;
    case Media_Format_Y16U:
        format = Format_Y16U;
        break;
    case Media_Format_VYUY:
        format = Format_VYUY;
        break;
    case Media_Format_YVYU:
        format = Format_YVYU;
        break;
    case Media_Format_A16R16G16B16:
        format = Format_A16R16G16B16;
        break;
    case Media_Format_A16B16G16R16:
        format = Format_A16B16G16R16;
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
                    MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->pHDRParams);
                    if (pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->p3DLutParams)
                    {
                        MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->p3DLutParams->pExt3DLutSurface);
                    }
                    MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pSrc[uSurfIndex]->p3DLutParams);
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
                if (pTarget->pHDRParams)
                {
                    MOS_FreeMemAndSetNull(pTarget->pHDRParams);
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
            MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pCompAlpha);
            if (nullptr != pVpCtx->pVpHalRenderParams->pColorFillParams)
            {
                MOS_FreeMemAndSetNull(pVpCtx->pVpHalRenderParams->pColorFillParams);
            }
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
        pVpCtx->MosDrvCtx.m_skuTable.reset();
        pVpCtx->MosDrvCtx.m_waTable.reset();
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
//! \purpose Check whether there is only Procamp with adjusting Brightness
//! \params
//! [in]  pVpHalSrcSurf
//! [out] None
//! \returns true if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////
bool IsProcmpEnable(PVPHAL_SURFACE pVpHalSrcSurf)
{
    DDI_CHK_NULL(pVpHalSrcSurf, "Null pVpHalSrcSurf.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if ((pVpHalSrcSurf->pProcampParams && pVpHalSrcSurf->pProcampParams->bEnabled) &&
        (pVpHalSrcSurf->pProcampParams->fContrast == 1 && pVpHalSrcSurf->pProcampParams->fHue == 0 && pVpHalSrcSurf->pProcampParams->fSaturation == 1) &&
        !pVpHalSrcSurf->pBlendingParams && !pVpHalSrcSurf->pLumaKeyParams && (!pVpHalSrcSurf->pIEFParams || !pVpHalSrcSurf->pIEFParams->bEnabled) &&
        !pVpHalSrcSurf->pDeinterlaceParams && (!pVpHalSrcSurf->pDenoiseParams || (!pVpHalSrcSurf->pDenoiseParams->bEnableChroma && !pVpHalSrcSurf->pDenoiseParams->bEnableLuma)) &&
        (!pVpHalSrcSurf->pColorPipeParams || (!pVpHalSrcSurf->pColorPipeParams->bEnableACE && !pVpHalSrcSurf->pColorPipeParams->bEnableSTE && !pVpHalSrcSurf->pColorPipeParams->bEnableTCC)) &&
        !pVpHalSrcSurf->pHDRParams)
    {
        return true;
    }

    return false;
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
//! \purpose Set the appropriate HDR params according to colour standard, HDR metadata.
//! \params
//! [in]  pVpHalSurf : VPHAL Surface
//! [in]  pHdrMetadata: HDR metadata
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////
VAStatus VpUpdateProcHdrState(
    const PVPHAL_SURFACE               pVpHalSurf,
    const VAHdrMetaData*               pHdrMetadata)
{
    DDI_CHK_NULL(pVpHalSurf, "Null pVpHalSurf.", VA_STATUS_ERROR_INVALID_BUFFER);

    // pass HDR metadata
    if ((pHdrMetadata != nullptr) && (pHdrMetadata->metadata_size != 0))
    {
        if (pVpHalSurf->pHDRParams == nullptr)
        {
            pVpHalSurf->pHDRParams = (PVPHAL_HDR_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_HDR_PARAMS));
            DDI_CHK_NULL(pVpHalSurf->pHDRParams, "VPHAL_HDR_PARAMS MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        // HDR10 Meta Data
        if (pHdrMetadata->metadata_type == VAProcHighDynamicRangeMetadataHDR10)
        {
            VP_DDI_NORMALMESSAGE("VpSetHdrParams HDR10 metadata.");
            VAHdrMetaDataHDR10* pHDR10MetaData = (VAHdrMetaDataHDR10*)pHdrMetadata->metadata;
            if (pHDR10MetaData)
            {
                pVpHalSurf->pHDRParams->white_point_x = pHDR10MetaData->white_point_x;
                pVpHalSurf->pHDRParams->white_point_y = pHDR10MetaData->white_point_y;
                VP_DDI_NORMALMESSAGE("pHDR10MetaData white_point_x %d, white_point_y %d.", pHDR10MetaData->white_point_x, pHDR10MetaData->white_point_y);

                // From VAAPI defintion which is following video spec, max/min_display_mastering_luminance are in units of 0.0001 candelas per square metre.
                uint32_t max_display_mastering_luminance = (pHDR10MetaData->max_display_mastering_luminance > 655350000 ) ? 655350000 : pHDR10MetaData->max_display_mastering_luminance;
                uint32_t min_display_mastering_luminance = (pHDR10MetaData->min_display_mastering_luminance > 655350000 ) ? 655350000 : pHDR10MetaData->min_display_mastering_luminance;
                pVpHalSurf->pHDRParams->max_display_mastering_luminance = (uint16_t)(max_display_mastering_luminance / 10000);
                pVpHalSurf->pHDRParams->min_display_mastering_luminance = (uint16_t)(min_display_mastering_luminance / 10000);
                VP_DDI_NORMALMESSAGE("pHDR10MetaData max_display_mastering_luminance %d, min_display_mastering_luminance %d.", pHDR10MetaData->max_display_mastering_luminance, pHDR10MetaData->min_display_mastering_luminance);

                pVpHalSurf->pHDRParams->MaxCLL  = pHDR10MetaData->max_content_light_level;
                pVpHalSurf->pHDRParams->MaxFALL = pHDR10MetaData->max_pic_average_light_level;
                VP_DDI_NORMALMESSAGE("pHDR10MetaData MaxCLL %d, MaxFALL %d.", pHDR10MetaData->max_content_light_level, pHDR10MetaData->max_pic_average_light_level);

                pVpHalSurf->pHDRParams->bAutoMode = false;

                pVpHalSurf->pHDRParams->MaxCLL  = (pVpHalSurf->pHDRParams->MaxCLL == 0) ? HDR_DEFAULT_MAXCLL : pVpHalSurf->pHDRParams->MaxCLL;
                pVpHalSurf->pHDRParams->MaxFALL = (pVpHalSurf->pHDRParams->MaxFALL == 0) ? HDR_DEFAULT_MAXFALL : pVpHalSurf->pHDRParams->MaxFALL;

                MOS_SecureMemcpy(pVpHalSurf->pHDRParams->display_primaries_x, 3 * sizeof(uint16_t), pHDR10MetaData->display_primaries_x, 3 * sizeof(uint16_t));
                MOS_SecureMemcpy(pVpHalSurf->pHDRParams->display_primaries_y, 3 * sizeof(uint16_t), pHDR10MetaData->display_primaries_y, 3 * sizeof(uint16_t));

                switch (pVpHalSurf->GammaType)
                {
                case VPHAL_GAMMA_SMPTE_ST2084:
                    pVpHalSurf->pHDRParams->EOTF = VPHAL_HDR_EOTF_SMPTE_ST2084;
                    break;
                case VPHAL_GAMMA_BT1886:
                    pVpHalSurf->pHDRParams->EOTF = VPHAL_HDR_EOTF_BT1886;
                    break;
                default:
                    pVpHalSurf->pHDRParams->EOTF = VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR;
                    break;
                }
                VP_DDI_NORMALMESSAGE("max_display_mastering_luminance %d.", pVpHalSurf->pHDRParams->max_display_mastering_luminance);
                VP_DDI_NORMALMESSAGE("min_display_mastering_luminance %d.", pVpHalSurf->pHDRParams->min_display_mastering_luminance);
                VP_DDI_NORMALMESSAGE("GammaType %d.", pVpHalSurf->GammaType);
            }
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

    if (IsProcmpEnable(pVpHalTgtSurf))
    {
        // correct the ChromaSitting location if Procamp is enabled.
#if (VA_MAJOR_VERSION < 1)
        pPipelineParam->output_surface_flag = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
#else
        pPipelineParam->output_color_properties.chroma_sample_location = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
#endif
    }

#if (VA_MAJOR_VERSION < 1)
    VpUpdateProcChromaSittingState(pVpHalTgtSurf, (uint8_t)(pPipelineParam->output_surface_flag & 0xff));
#else
    VpUpdateProcChromaSittingState(pVpHalTgtSurf, pPipelineParam->output_color_properties.chroma_sample_location);
#endif
    return VA_STATUS_SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Set Interpolation Method according to the flag
//! \params
//! [in]  PVPHAL_SURFACE : VA Surface
//! [in]  uInterpolationflags : Interpolation Flag
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
VpSetInterpolationParams(
    PVPHAL_SURFACE                  pSurface,
    uint32_t                        uInterpolationflags)
{
    DDI_CHK_NULL(pSurface, "Null pSurface.", VA_STATUS_ERROR_INVALID_SURFACE);
    switch (uInterpolationflags)
    {
#if VA_CHECK_VERSION(1, 9, 0)
    case VA_FILTER_INTERPOLATION_NEAREST_NEIGHBOR:
        pSurface->ScalingMode       = VPHAL_SCALING_NEAREST;
        break;
    case VA_FILTER_INTERPOLATION_BILINEAR:
        pSurface->ScalingMode       = VPHAL_SCALING_BILINEAR;
        break;
    case VA_FILTER_INTERPOLATION_ADVANCED:
    case VA_FILTER_INTERPOLATION_DEFAULT:
#endif
    default:
        pSurface->ScalingMode       = VPHAL_SCALING_AVS;
        break;
    }

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
    uint32_t                    uInterpolationflags;
    uint32_t                    uChromaSitingFlags;
    VAStatus                    vaStatus;
    MOS_STATUS                  eStatus;
    DDI_VP_STATE                vpStateFlags = {};
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
    uInterpolationflags = 0;

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

    pVpHalSrcSurf->Format   = VpGetFormatFromMediaFormat(pMediaSrcSurf->format);
    pVpHalSrcSurf->TileType = VpGetTileTypeFromMediaTileType(pMediaSrcSurf->TileType);
    
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

    // Currently we only support 1 primary surface in VP
    if (pVpCtx->iPriSurfs < VP_MAX_PRIMARY_SURFS)
    {
        pVpHalSrcSurf->SurfType = SURF_IN_PRIMARY;
        pVpCtx->iPriSurfs++;
    }
    else
    {
        pVpHalSrcSurf->SurfType = SURF_IN_SUBSTREAM;
    }
    
    // Set workload path using pipeline_flags VA_PROC_PIPELINE_FAST flag
    if (pPipelineParam->pipeline_flags & VA_PROC_PIPELINE_FAST)
    {
        pVpHalRenderParams->bForceToRender = true;
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

    if ((pVpHalTgtSurf->rcSrc.right < pVpHalSrcSurf->rcDst.right) ||
        (pVpHalTgtSurf->rcSrc.bottom < pVpHalSrcSurf->rcDst.bottom))
    {
        DDI_CHK_CONDITION(true, "Invalid color fill parameter!", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    //set the frame_id
    pVpHalSrcSurf->FrameID = pMediaSrcSurf->frame_idx;

    //In order to refresh the frame ID in DdiVp_UpdateFilterParamBuffer when run ADI case,
    //we need to set the OS resource here.
    pVpHalSrcSurf->OsResource.bo          = pMediaSrcSurf->bo;

    // csc option
    //---------------------------------------
    // Set color space for src
#if (VA_MAJOR_VERSION < 1)
    vaStatus = DdiVp_GetColorSpace(pVpHalSrcSurf, pPipelineParam->surface_color_standard, pPipelineParam->input_surface_flag);
#else
    vaStatus = DdiVp_GetColorSpace(pVpHalSrcSurf, pPipelineParam->surface_color_standard, pPipelineParam->input_color_properties);
#endif
    DDI_CHK_RET(vaStatus, "Unsupport Color space!");

    if(pMediaSrcSurf->format == Media_Format_400P)
    {
        pVpHalSrcSurf->ColorSpace = CSpace_BT601Gray;
    }

    // extended gamut? RGB can't have extended gamut flag
    pVpHalSrcSurf->ExtendedGamut = false;

    // Background Colorfill
    // According to libva  definition, if alpha in output background color is zero, then colorfill is not needed
    if ((pPipelineParam->output_background_color >> 24) != 0 || pVpHalTgtSurf->ColorSpace == CSpace_sRGB)
    {
        if (pVpHalRenderParams->pColorFillParams == nullptr)
        {
            pVpHalRenderParams->pColorFillParams = (PVPHAL_COLORFILL_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_COLORFILL_PARAMS));
        }

        DDI_CHK_NULL(pVpHalRenderParams->pColorFillParams, "Null pColorFillParams.", VA_STATUS_ERROR_UNKNOWN);

        if (pVpHalTgtSurf->ColorSpace == CSpace_sRGB && (pPipelineParam->output_background_color >> 24) == 0)
        {
            // set color space for sRGB output
            pVpHalRenderParams->pColorFillParams->CSpace    = CSpace_sRGB;
        }
        else
        {
            // set background colorfill option
            pVpHalRenderParams->pColorFillParams->Color     = pPipelineParam->output_background_color;
            pVpHalRenderParams->pColorFillParams->bYCbCr    = false;
            pVpHalRenderParams->pColorFillParams->CSpace    = CSpace_sRGB;
        }
    }else
    {
        MOS_FreeMemAndSetNull(pVpHalRenderParams->pColorFillParams);
    }

    // Set Demo Mode option
    if (pVpHalRenderParams->bDisableDemoMode == false)
    {
        eStatus = VpHal_DdiSetupSplitScreenDemoMode(
            0,  // No DDI setting on Linux. Set it when Linux DDI supports it
            0,  // No DDI setting on Linux. Set it when Linux DDI supports it
            &pVpHalRenderParams->pSplitScreenDemoModeParams,
            &pVpHalRenderParams->bDisableDemoMode,
            pOsInterface);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            VP_DDI_ASSERTMESSAGE("Failed to setup Split-Screen Demo Mode.");
            MOS_FreeMemAndSetNull(pVpHalRenderParams->pSplitScreenDemoModeParams);
        }
    }

    // Update fwd and bkward ref frames: Required for Advanced processing - will be supported in the future

    pVpHalSrcSurf->uFwdRefCount  = pPipelineParam->num_backward_references;

    vaStatus = DdiVp_UpdateProcPipelineFutureReferenceFrames(pVpCtx, pVaDrvCtx, pVpHalSrcSurf, pPipelineParam);
    DDI_CHK_RET(vaStatus, "Failed to update future  reference frames!");

    pVpHalSrcSurf->uBwdRefCount  = pPipelineParam->num_forward_references;

    vaStatus = DdiVp_UpdateProcPipelinePastReferenceFrames(pVpCtx, pVaDrvCtx, pVpHalSrcSurf, pPipelineParam);
    DDI_CHK_RET(vaStatus, "Failed to update past reference frames!");

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
            vaStatus = DdiVp_UpdateFilterParamBuffer(pVaDrvCtx, pVpCtx, uSurfIndex, filter_param->type, pData, pFilterBuf->uiNumElements, &vpStateFlags);
            DDI_CHK_RET(vaStatus, "Failed to update parameter buffer!");
        }
    }

    DdiVp_ClearFilterParamBuffer(pVpCtx, uSurfIndex, vpStateFlags);

    // Update the Deinterlace params
    //vaStatus = DdiVp_UpdateProcDeinterlaceParams(pVaDrvCtx, pVpHalSrcSurf, pPipelineParam);
    //DDI_CHK_RET(vaStatus, "Failed to update vphal advance deinterlace!");

    // Use Render to do scaling for resolution larger than 8K
    if(MEDIA_IS_WA(&pMediaCtx->WaTable, WaDisableVeboxFor8K))
    {
        pVpHalRenderParams->bDisableVeboxFor8K = true;
    }

    // Scaling algorithm
    uScalingflags                    = pPipelineParam->filter_flags & VA_FILTER_SCALING_MASK;
    pVpHalSrcSurf->ScalingPreference = VPHAL_SCALING_PREFER_SFC;    // default
    // Interpolation method
#if VA_CHECK_VERSION(1, 9, 0)
    uInterpolationflags              = pPipelineParam->filter_flags & VA_FILTER_INTERPOLATION_MASK;
#endif
    switch (uScalingflags)
    {
    case VA_FILTER_SCALING_FAST:
        if (pVpHalSrcSurf->SurfType == SURF_IN_PRIMARY)
        {
            VpSetInterpolationParams(pVpHalSrcSurf, uInterpolationflags);
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
            VpSetInterpolationParams(pVpHalSrcSurf, uInterpolationflags);
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
            VpSetInterpolationParams(pVpHalSrcSurf, uInterpolationflags);
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
    pVpHalSrcSurf->bFieldWeaving      = false;

    if (pVpHalSrcSurf->pDeinterlaceParams == nullptr)
    {
        if (pPipelineParam->input_surface_flag & VA_TOP_FIELD_FIRST)
        {
            if (pPipelineParam->output_surface_flag & VA_TOP_FIELD_FIRST)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
                pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
                pVpHalSrcSurf->bInterlacedScaling = true;
                pVpHalSrcSurf->bFieldWeaving = false;
            }else if (pPipelineParam->output_surface_flag & VA_TOP_FIELD)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_FIELD;
                pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
                pVpHalTgtSurf->SampleType = SAMPLE_SINGLE_TOP_FIELD;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }else if (pPipelineParam->output_surface_flag & VA_BOTTOM_FIELD)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_FIELD;
                pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;
                pVpHalTgtSurf->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }else
            {
                VP_DDI_ASSERTMESSAGE("output_surface_flag need to be set for interlaced scaling.");
                pVpHalSrcSurf->SampleType = SAMPLE_PROGRESSIVE;
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_NONE;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }
        }
        else if (pPipelineParam->input_surface_flag & VA_BOTTOM_FIELD_FIRST)
        {
            if (pPipelineParam->output_surface_flag & VA_BOTTOM_FIELD_FIRST)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
                pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
                pVpHalSrcSurf->bInterlacedScaling = true;
                pVpHalSrcSurf->bFieldWeaving = false;
            }else if (pPipelineParam->output_surface_flag & VA_TOP_FIELD)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_FIELD;
                pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD;
                pVpHalTgtSurf->SampleType = SAMPLE_SINGLE_TOP_FIELD;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }else if (pPipelineParam->output_surface_flag & VA_BOTTOM_FIELD)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_FIELD;
                pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
                pVpHalTgtSurf->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }else
            {
                VP_DDI_ASSERTMESSAGE("output_surface_flag need to be set for interlaced scaling.");
                pVpHalSrcSurf->SampleType = SAMPLE_PROGRESSIVE;
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_NONE;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }
        }else if (pPipelineParam->input_surface_flag & VA_TOP_FIELD)
        {
            if (pPipelineParam->output_surface_flag & VA_TOP_FIELD_FIRST)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_FIELD_TO_INTERLEAVED;
                pVpHalSrcSurf->SampleType = SAMPLE_SINGLE_TOP_FIELD;
                pVpHalTgtSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;

                DDI_CHK_NULL(pVpHalSrcSurf->pBwdRef, "No Ref Field!", VA_STATUS_ERROR_UNIMPLEMENTED);
                pVpHalSrcSurf->pBwdRef->InterlacedScalingType = ISCALING_FIELD_TO_INTERLEAVED;
                pVpHalSrcSurf->pBwdRef->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;

                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = true;
            }else if(pPipelineParam->output_surface_flag & VA_TOP_FIELD)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_FIELD_TO_FIELD;
                pVpHalSrcSurf->SampleType = SAMPLE_SINGLE_TOP_FIELD;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }else
            {
                pVpHalSrcSurf->SampleType = SAMPLE_PROGRESSIVE;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }
        }else if (pPipelineParam->input_surface_flag & VA_BOTTOM_FIELD)
        {
            if (pPipelineParam->output_surface_flag & VA_BOTTOM_FIELD_FIRST)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_FIELD_TO_INTERLEAVED;
                pVpHalSrcSurf->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;
                pVpHalTgtSurf->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;

                DDI_CHK_NULL(pVpHalSrcSurf->pBwdRef, "No Ref Field!", VA_STATUS_ERROR_UNIMPLEMENTED);
                pVpHalSrcSurf->pBwdRef->InterlacedScalingType = ISCALING_FIELD_TO_INTERLEAVED;
                pVpHalSrcSurf->pBwdRef->SampleType = SAMPLE_SINGLE_TOP_FIELD;

                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = true;
            }else if(pPipelineParam->output_surface_flag & VA_BOTTOM_FIELD)
            {
                pVpHalSrcSurf->InterlacedScalingType = ISCALING_FIELD_TO_FIELD;
                pVpHalSrcSurf->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }else
            {
                pVpHalSrcSurf->SampleType = SAMPLE_PROGRESSIVE;
                pVpHalSrcSurf->bInterlacedScaling = false;
                pVpHalSrcSurf->bFieldWeaving = false;
            }
        }else
        {
            pVpHalSrcSurf->SampleType = SAMPLE_PROGRESSIVE;
            pVpHalSrcSurf->InterlacedScalingType = ISCALING_NONE;
            pVpHalSrcSurf->bInterlacedScaling = false;
            pVpHalSrcSurf->bFieldWeaving = false;
        }
    }

    // For legacy interlace scaling
    if (pVpHalSrcSurf->pDeinterlaceParams == nullptr && pVpHalSrcSurf->InterlacedScalingType == ISCALING_NONE)
    {
        if (pPipelineParam->filter_flags & VA_TOP_FIELD)
        {
            pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
            pVpHalSrcSurf->ScalingMode = VPHAL_SCALING_AVS;
            pVpHalSrcSurf->bInterlacedScaling = true;
            pVpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
        }
        else if (pPipelineParam->filter_flags & VA_BOTTOM_FIELD)
        {
            pVpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
            pVpHalSrcSurf->ScalingMode = VPHAL_SCALING_AVS;
            pVpHalSrcSurf->bInterlacedScaling = true;
            pVpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
        }

        // Kernel does not support 3-plane interlaced AVS, so for 3-plane interlaced scaling, need to use bilinear.
        if (pVpHalSrcSurf->bInterlacedScaling && IS_PL3_FORMAT(pVpHalSrcSurf->Format))
        {
            pVpHalSrcSurf->ScalingMode = VPHAL_SCALING_BILINEAR;
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

    if (IsProcmpEnable(pVpHalSrcSurf))
    {
        // correct the ChromaSitting location if Procamp is enabled.
#if (VA_MAJOR_VERSION < 1)
        pPipelineParam->input_surface_flag = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
        pPipelineParam->output_surface_flag = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
#else
        pPipelineParam->input_color_properties.chroma_sample_location = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
        pPipelineParam->output_color_properties.chroma_sample_location = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
#endif
    }

#if (VA_MAJOR_VERSION < 1)
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
    vaStatus = DdiVp_UpdateVphalTargetSurfColorSpace(pVaDrvCtx, pVpCtx, pPipelineParam, 0);
    DDI_CHK_RET(vaStatus, "Failed to update vphal target surface color space!");
    // Update the Render Target HDR params - this needs to be done once when Render Target is passed via BeginPicture
    vaStatus = VpUpdateProcHdrState(pVpHalTgtSurf, pPipelineParam->output_hdr_metadata);
    DDI_CHK_RET(vaStatus, "Failed to update vphal target surface HDR metadata!");

    // Using additional_outputs processing as 1:N case.
    for (i = 0; i < pPipelineParam->num_additional_outputs; i++)
    {
        vaStatus = DdiVp_BeginPictureInt(pVaDrvCtx, pVpCtx, pPipelineParam->additional_outputs[i]);
        DDI_CHK_RET(vaStatus, "Failed to update vphal target surface buffers!");
        vaStatus = DdiVp_UpdateVphalTargetSurfColorSpace(pVaDrvCtx, pVpCtx, pPipelineParam, i+1);
        DDI_CHK_RET(vaStatus, "Failed to update vphal target surface color space!");
    }

    // add 16aligned UsrPtr mode support
    if (pMediaSrcSurf->pSurfDesc && (pMediaSrcSurf->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR))
    {
        pVpHalSrcSurf->b16UsrPtr = VpIs16UsrPtrPitch(pMediaSrcSurf->iPitch, pMediaSrcSurf->format);
    }
    else
    {
        pVpHalSrcSurf->b16UsrPtr = false;
    }

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
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

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
    pVpCtx->MosDrvCtx.bufmgr          = pMediaCtx->pDrmBufMgr;
    pVpCtx->MosDrvCtx.m_gpuContextMgr = pMediaCtx->m_gpuContextMgr;
    pVpCtx->MosDrvCtx.m_cmdBufMgr     = pMediaCtx->m_cmdBufMgr;
    pVpCtx->MosDrvCtx.fd              = pMediaCtx->fd;
    pVpCtx->MosDrvCtx.iDeviceId       = pMediaCtx->iDeviceId;
    pVpCtx->MosDrvCtx.m_skuTable      = pMediaCtx->SkuTable;
    pVpCtx->MosDrvCtx.m_waTable       = pMediaCtx->WaTable;
    pVpCtx->MosDrvCtx.m_gtSystemInfo  = *pMediaCtx->pGtSystemInfo;
    pVpCtx->MosDrvCtx.m_platform      = pMediaCtx->platform;
    pVpCtx->MosDrvCtx.m_auxTableMgr   = pMediaCtx->m_auxTableMgr;
    pVpCtx->MosDrvCtx.pGmmClientContext = pMediaCtx->pGmmClientContext;
    pVpCtx->MosDrvCtx.ppMediaMemDecompState = &pMediaCtx->pMediaMemDecompState;
    pVpCtx->MosDrvCtx.pfnMediaMemoryCopy    = pMediaCtx->pfnMediaMemoryCopy;
    pVpCtx->MosDrvCtx.pfnMediaMemoryCopy2D  = pMediaCtx->pfnMediaMemoryCopy2D;
    pVpCtx->MosDrvCtx.pfnMemoryDecompress   = pMediaCtx->pfnMemoryDecompress;
    pVpCtx->MosDrvCtx.ppMediaCopyState      = &pMediaCtx->pMediaCopyState;

    pVpCtx->MosDrvCtx.m_osDeviceContext     = pMediaCtx->m_osDeviceContext;
    pVpCtx->MosDrvCtx.m_apoMosEnabled       = pMediaCtx->m_apoMosEnabled;
    pVpCtx->MosDrvCtx.m_userSettingPtr      = pMediaCtx->m_userSettingPtr;

    pVpCtx->MosDrvCtx.pPerfData = (PERF_DATA *)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));
    if (nullptr == pVpCtx->MosDrvCtx.pPerfData)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    // initialize DDI level cp interface
    pVpCtx->pCpDdiInterface = Create_DdiCpInterface(pVpCtx->MosDrvCtx);
    if (nullptr == pVpCtx->pCpDdiInterface)
    {
         vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
         goto FINISH;
    }


    DDI_CHK_RET(DdiVp_InitVpHal(pVpCtx), "Call DdiVp_InitVpHal failed");

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
        Delete_DdiCpInterface(pVpCtx->pCpDdiInterface);
        pVpCtx->pCpDdiInterface = NULL;
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
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    VpBase                    *pVpHal;
    VphalSettings             VpHalSettings;

    VAStatus                  vaStatus;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    vaStatus            = VA_STATUS_ERROR_UNKNOWN;
    pVpHal              = nullptr;

    // Create VpHal state
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;
    pVpHal = VpBase::VphalStateFactory( nullptr, &(pVpCtx->MosDrvCtx), &eStatus);

    if (pVpHal && MOS_FAILED(eStatus))
    {
        MOS_Delete(pVpHal);
        pVpHal = nullptr;
    }

    if (!pVpHal)
    {
        VP_DDI_ASSERTMESSAGE("Failed to create vphal.");
        MOS_Delete(pVpCtx);
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

    return VA_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
////! \purpose Set Color Standard Explictly.
////! \params
////! [in]  pVpHalSurf : src/target surface
////! [in]  colorStandard : VA color standard VAProcColorStandardType
////! [in]  colorProperties : input/output surface color properties
////! [out] None
////! \returns appropriate VA_STATUS_SUCCESS if call succeeds
///////////////////////////////////////////////////////////////////////////////////////////////
VAStatus VpSetColorStandardExplictly(PVPHAL_SURFACE pVpHalSurf, VAProcColorStandardType colorStandard, VAProcColorProperties colorProperties)
{
    DDI_CHK_NULL(pVpHalSurf, "Null pVpHalSurf.", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_CONDITION((colorStandard != VAProcColorStandardExplicit), "Not Explict color standard, Exit!", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (IS_RGB_FORMAT(pVpHalSurf->Format))
    {
        switch(colorProperties.colour_primaries)
        {
        case COLOUR_PRIMARY_BT2020:
            pVpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_REDUCED) ? CSpace_BT2020_stRGB : CSpace_BT2020_RGB;
            break;
        case COLOUR_PRIMARY_BT709:
        case COLOUR_PRIMARY_BT601:
            pVpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_REDUCED) ? CSpace_stRGB: CSpace_sRGB;
            break;
        default:
            pVpHalSurf->ColorSpace = CSpace_sRGB;
            VP_DDI_ASSERTMESSAGE("unknown Color Standard for RGB format.");
            break;
        }
    }

    if (IS_YUV_FORMAT(pVpHalSurf->Format) || IS_ALPHA_YUV_FORMAT(pVpHalSurf->Format))
    {
        switch(colorProperties.colour_primaries)
        {
        case COLOUR_PRIMARY_BT2020:
            pVpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_FULL) ? CSpace_BT2020_FullRange : CSpace_BT2020;
            break;
        case COLOUR_PRIMARY_BT709:
            pVpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_FULL) ? CSpace_BT709_FullRange : CSpace_BT709;
            break;
        case COLOUR_PRIMARY_BT601:
            pVpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_FULL) ? CSpace_BT601_FullRange : CSpace_BT601;
            break;
        default:
            pVpHalSurf->ColorSpace = CSpace_BT601;
            VP_DDI_ASSERTMESSAGE("unknown Color Standard for YUV format.");
            break;
        }
    }

    switch(colorProperties.transfer_characteristics)
    {
    case TRANSFER_CHARACTERISTICS_ST2084:
        pVpHalSurf->GammaType = VPHAL_GAMMA_SMPTE_ST2084;
        break;
    default:
        pVpHalSurf->GammaType = VPHAL_GAMMA_TRADITIONAL_GAMMA;
        break;
    }

    return VA_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////
////! \purpose Convert VAProcColorStandardType to VPHAL_CSPACE
////! \params
////! [in]  pVpHalSurf : src/target surface
////! [in]  colorStandard : VA color standard VAProcColorStandardType
////! [in]  flag : input/output surface flag for full/reduced color range
////! [out] None
////! \returns appropriate VPHAL_CSPACE if call succeeds
///////////////////////////////////////////////////////////////////////////////////////////////
#if (VA_MAJOR_VERSION < 1)
VAStatus DdiVp_GetColorSpace(PVPHAL_SURFACE pVpHalSurf, VAProcColorStandardType colorStandard, uint32_t flag)
#else
VAStatus DdiVp_GetColorSpace(PVPHAL_SURFACE pVpHalSurf, VAProcColorStandardType colorStandard, VAProcColorProperties colorProperties)
#endif
{
    uint8_t color_range = colorProperties.color_range;

    pVpHalSurf->ColorSpace = CSpace_None;

    VP_DDI_FUNCTION_ENTER;

    // Convert VAProcColorStandardType to VPHAL_CSPACE
    if (IS_RGB_FORMAT(pVpHalSurf->Format) || (pVpHalSurf->Format == Format_P8))
    {
        switch (colorStandard)
        {
            case VAProcColorStandardBT2020:
#if (VA_MAJOR_VERSION < 1)
                if (flag & VA_SOURCE_RANGE_FULL)
#else
                if (color_range == VA_SOURCE_RANGE_FULL)
#endif
                {
                    pVpHalSurf->ColorSpace = CSpace_BT2020_RGB;
                }
                else
                {
                    pVpHalSurf->ColorSpace = CSpace_BT2020_stRGB;
                }
                break;
            case VAProcColorStandardSTRGB:
                pVpHalSurf->ColorSpace = CSpace_stRGB;
                break;
            case VAProcColorStandardExplicit:
                VpSetColorStandardExplictly(pVpHalSurf, colorStandard, colorProperties);
                break;
            case VAProcColorStandardSRGB:
            default:
                pVpHalSurf->ColorSpace = CSpace_sRGB;
                break;
        }
    }
    else
    {
        // Set colorspace by default to avoid application don't set ColorStandard
        if (colorStandard == 0)
        {
            if ((pVpHalSurf->rcSrc.right - pVpHalSurf->rcSrc.left) <= 1280 && (pVpHalSurf->rcSrc.bottom - pVpHalSurf->rcSrc.top) <= 720)
            {
                pVpHalSurf->ColorSpace = CSpace_BT601;
            }//720p
            else if ((pVpHalSurf->rcSrc.right - pVpHalSurf->rcSrc.left) <= 1920 && (pVpHalSurf->rcSrc.bottom - pVpHalSurf->rcSrc.top) <= 1080)
            {
                pVpHalSurf->ColorSpace = CSpace_BT709;
            }//1080p
            else
            {
                if (pVpHalSurf->Format == Format_P010 || pVpHalSurf->Format == Format_P016)
                {
                    pVpHalSurf->ColorSpace = CSpace_BT2020;
                }
                else
                {
                    pVpHalSurf->ColorSpace = CSpace_BT709;
                }
            }//4K
        }
        else
        {
            switch (colorStandard)
            {
                case VAProcColorStandardBT709:
#if (VA_MAJOR_VERSION < 1)
                    if (flag & VA_SOURCE_RANGE_FULL)
#else
                    if (color_range == VA_SOURCE_RANGE_FULL)
#endif
                    {
                        pVpHalSurf->ColorSpace = CSpace_BT709_FullRange;
                    }
                    else
                    {
                        pVpHalSurf->ColorSpace = CSpace_BT709;
                    }
                    break;
                case VAProcColorStandardBT601:
#if (VA_MAJOR_VERSION < 1)
                    if (flag & VA_SOURCE_RANGE_FULL)
#else
                    if (color_range == VA_SOURCE_RANGE_FULL)
#endif
                    {
                        pVpHalSurf->ColorSpace = CSpace_BT601_FullRange;
                    }
                    else
                    {
                        pVpHalSurf->ColorSpace = CSpace_BT601;
                    }
                    break;
                case VAProcColorStandardBT2020:
#if (VA_MAJOR_VERSION < 1)
                    if (flag & VA_SOURCE_RANGE_FULL)
 #else
                    if (color_range == VA_SOURCE_RANGE_FULL)
 #endif
                    {
                        pVpHalSurf->ColorSpace = CSpace_BT2020_FullRange;
                    }
                    else
                    {
                        pVpHalSurf->ColorSpace = CSpace_BT2020;
                    }
                    break;
                case VAProcColorStandardBT470M:
                case VAProcColorStandardBT470BG:
                case VAProcColorStandardSMPTE170M:
                case VAProcColorStandardSMPTE240M:
                case VAProcColorStandardGenericFilm:
                case VAProcColorStandardXVYCC601:
                case VAProcColorStandardXVYCC709:
                    pVpHalSurf->ColorSpace == CSpace_None;
                    break;
                case VAProcColorStandardExplicit:
                    VpSetColorStandardExplictly(pVpHalSurf, colorStandard, colorProperties);
                    break;
                default:
                    pVpHalSurf->ColorSpace == CSpace_BT601;
                    break;
            }
        }
    }
    DDI_CHK_CONDITION((pVpHalSurf->ColorSpace == CSpace_None), "Invalid color standard", VA_STATUS_ERROR_INVALID_PARAMETER);

    return VA_STATUS_SUCCESS;
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
    MOS_FORMAT mosFormat = VpGetFormatFromMediaFormat(format);

    if (IS_RGB_FORMAT(mosFormat))
    {
        return CSpace_sRGB;
    }
    else
    {
        return CSpace_BT601;
    }
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
    VAProcPipelineParameterBuffer*  pPipelineParam,
    uint32_t                        targetIndex)
{
    PVPHAL_RENDER_PARAMS      pVpHalRenderParams;
    PVPHAL_SURFACE            pVpHalSrcSurf;
    PVPHAL_SURFACE            pVpHalTgtSurf;
    VAStatus                  vaStatus;
    DDI_UNUSED(pVaDrvCtx);

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pVpHalTgtSurf      = pVpHalRenderParams->pTarget[targetIndex];
    DDI_CHK_NULL(pVpHalTgtSurf, "Null pVpHalTgtSurf.", VA_STATUS_ERROR_INVALID_SURFACE);

    // update target surface color space
#if (VA_MAJOR_VERSION < 1)
    vaStatus = DdiVp_GetColorSpace(pVpHalTgtSurf, pPipelineParam->output_color_standard, pPipelineParam->output_surface_flag);
#else
    vaStatus = DdiVp_GetColorSpace(pVpHalTgtSurf, pPipelineParam->output_color_standard, pPipelineParam->output_color_properties);
#endif

    pVpHalSrcSurf = pVpHalRenderParams->pSrc[0];
    // Not support BT601/BT709 -> BT2020 colorspace conversion, if colorspace is not set, will keep it same with input.
    if(pVpHalSrcSurf != nullptr &&
       pPipelineParam->output_color_standard == 0 &&
       IS_COLOR_SPACE_BT2020(pVpHalTgtSurf->ColorSpace) &&
       !IS_COLOR_SPACE_BT2020(pVpHalSrcSurf->ColorSpace))
    {
        pVpHalTgtSurf->ColorSpace = pVpHalSrcSurf->ColorSpace;
    }

    // extended gamut?
    pVpHalRenderParams->pTarget[0]->ExtendedGamut = false;

    return vaStatus;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Setup the appropriate filter params for VPHAL input surface based on Filter type
//! \params
//! [in]  pVaDrvCtx : Driver context
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
        VADriverContextP    pVaDrvCtx,
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
        case VAProcFilterHVSNoiseReduction:
            vpStateFlags->bDenoiseEnable = true;
            vaStatus = DdiVp_SetProcFilterHVSDenoiseParams(
                                            pVpCtx,
                                            uSurfIndex,
                                            (VAProcFilterParameterBufferHVSNoiseReduction*) pData);
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
        case VAProcFilterHighDynamicRangeToneMapping:
            vaStatus = DdiVp_SetProcFilterHdrTmParams(
                                            pVpCtx,
                                            uSurfIndex,
                                            (VAProcFilterParameterBufferHDRToneMapping*) pData);
            break;
#if VA_CHECK_VERSION(1, 12, 0)
        case VAProcFilter3DLUT:
            vaStatus = DdiVp_SetProcFilter3DLutParams(
                                            pVaDrvCtx,
                                            pVpCtx,
                                            uSurfIndex,
                                            (VAProcFilterParameterBuffer3DLUT*) pData);
            break;
#endif
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
        case VAProcDeinterlacingMotionCompensated:
            DIMode = DI_MODE_ADI;
            break;
        case VAProcDeinterlacingWeave:
            pSrc->bFieldWeaving = true;;
            return VA_STATUS_SUCCESS;
        case VAProcDeinterlacingNone:
            return VA_STATUS_SUCCESS;
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
        pSrc->pDeinterlaceParams->bSCDEnable = true;
    }
    else
    {
        pSrc->pDeinterlaceParams->bSCDEnable = false;
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
DdiVp_SetProcFilterHVSDenoiseParams(
    PDDI_VP_CONTEXT                                  pVpCtx,
    uint32_t                                         uSurfIndex,
    VAProcFilterParameterBufferHVSNoiseReduction*    pHVSDnParamBuff)
{
    PVPHAL_RENDER_PARAMS      pVpHalRenderParams = nullptr;
    PVPHAL_SURFACE            pSrc = nullptr;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pHVSDnParamBuff,
                "Null pHVSDnParamBuff.",
                     VA_STATUS_ERROR_INVALID_PARAMETER);

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
    
    // Luma and chroma denoise should be always enabled when noise reduction is needed
    pSrc->pDenoiseParams->bEnableLuma       = true;
    pSrc->pDenoiseParams->bEnableChroma     = true;
    pSrc->pDenoiseParams->bEnableHVSDenoise = true;
    
    switch (pHVSDnParamBuff->mode)
    {
        case VA_PROC_HVS_DENOISE_AUTO_SUBJECTIVE:
            pSrc->pDenoiseParams->HVSDenoise.Mode = HVSDENOISE_AUTO_SUBJECTIVE;
            pSrc->pDenoiseParams->bAutoDetect     = true;
            break;
        case VA_PROC_HVS_DENOISE_MANUAL:
            pSrc->pDenoiseParams->HVSDenoise.Mode = HVSDENOISE_MANUAL;
            break;
        case VA_PROC_HVS_DENOISE_DEFAULT:
        case VA_PROC_HVS_DENOISE_AUTO_BDRATE:
        default:
            pSrc->pDenoiseParams->HVSDenoise.Mode = HVSDENOISE_AUTO_BDRATE;
            pSrc->pDenoiseParams->bAutoDetect     = true;
    }// switch (pHVSDnParamBuff->mode)

    if (pSrc->pDenoiseParams->HVSDenoise.Mode == HVSDENOISE_AUTO_BDRATE)
    {
        pSrc->pDenoiseParams->HVSDenoise.QP       = pHVSDnParamBuff->qp;
        if (pSrc->pDenoiseParams->HVSDenoise.QP == 0)
        {
            //If didn't set value, HVS Auto Bdrate Mode default qp 27
            pSrc->pDenoiseParams->HVSDenoise.QP   = 27;
        }
    }
    else if (pSrc->pDenoiseParams->HVSDenoise.Mode == HVSDENOISE_AUTO_SUBJECTIVE)
    {
        //HVS Subjective Mode default qp 32
        pSrc->pDenoiseParams->HVSDenoise.QP       = 32;
    }
    else
    {
        pSrc->pDenoiseParams->HVSDenoise.QP       = 32;
        pSrc->pDenoiseParams->HVSDenoise.Strength = pHVSDnParamBuff->strength;
    }

    VP_DDI_NORMALMESSAGE("HVS Denoise is enabled with qp %d, strength %d, mode %d!", pSrc->pDenoiseParams->HVSDenoise.QP, pSrc->pDenoiseParams->HVSDenoise.Strength, pSrc->pDenoiseParams->HVSDenoise.Mode);

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

/////////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose High Dynamic Range (HDR) Tone Mapping filter params for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  pHDRParamBuff : Pointer to High Dynamic Range Tone Mapping param buffer data
//! [in]  uElementNum : number of elements in the High Dynamic Range Tone Mapping param buffer data
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
//////////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcFilterHdrTmParams(
    PDDI_VP_CONTEXT                                  pVpCtx,
    uint32_t                                         uSurfIndex,
    VAProcFilterParameterBufferHDRToneMapping*       pHdrTmParamBuff)
{
    PVPHAL_RENDER_PARAMS pVpHalRenderParams  = nullptr;
    PVPHAL_SURFACE       pSrc = nullptr;
    VAStatus             eStatus;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(pHdrTmParamBuff, "Null pHdrTmParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pSrc = pVpHalRenderParams->pSrc[uSurfIndex];
    DDI_CHK_NULL(pSrc, "Null pSrc.", VA_STATUS_ERROR_INVALID_SURFACE);

    eStatus = VpUpdateProcHdrState(pSrc, &pHdrTmParamBuff->data);

    return eStatus;
}
#if VA_CHECK_VERSION(1, 12, 0)
/////////////////////////////////////////////////////////////////////////////////////////////////
//! \purpose Three Three-Dimensional Look Up Table(3DLUT) filter params for VPHAL input surface
//! \params
//! [in]  pVaDrvCtx: Driver context
//! [in]  pVpCtx : VP context
//! [in]  uSurfIndex : uSurfIndex to the input surface array
//! [in]  p3DLutParamBuff : Pointer to 3DLUT param buffer data
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
//////////////////////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_SetProcFilter3DLutParams(
    VADriverContextP                                 pVaDrvCtx,
    PDDI_VP_CONTEXT                                  pVpCtx,
    uint32_t                                         uSurfIndex,
    VAProcFilterParameterBuffer3DLUT*                p3DLutParamBuff)
{
    PVPHAL_RENDER_PARAMS pVpHalRenderParams  = nullptr;
    PVPHAL_SURFACE       pSrc                = nullptr;
    VAStatus             eStatus             = VA_STATUS_SUCCESS;

    PDDI_MEDIA_SURFACE   pMedia3DLutSurf     = nullptr;
    PDDI_MEDIA_CONTEXT   pMediaCtx           = nullptr;

    VP_DDI_FUNCTION_ENTER;
    DDI_CHK_NULL(p3DLutParamBuff, "Null p3DLutParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(pVaDrvCtx,       "Null pVaDrvCtx.",       VA_STATUS_ERROR_INVALID_BUFFER);

    pMediaCtx   = DdiMedia_GetMediaContext(pVaDrvCtx);

    pVpHalRenderParams = VpGetRenderParams(pVpCtx);
    DDI_CHK_NULL(pVpHalRenderParams, "Null pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    pSrc = pVpHalRenderParams->pSrc[uSurfIndex];
    DDI_CHK_NULL(pSrc, "Null pSrc.", VA_STATUS_ERROR_INVALID_SURFACE);

    pMedia3DLutSurf = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, p3DLutParamBuff->lut_surface);
    DDI_CHK_NULL(pMedia3DLutSurf, "Null pMedia3DLutSurf.", VA_STATUS_ERROR_INVALID_SURFACE);

    // only primary surface takes effect if 3DLUT filter
    if (SURF_IN_PRIMARY == pSrc->SurfType)
    {
        if (nullptr == pSrc->p3DLutParams)
        {
            pSrc->p3DLutParams = (PVPHAL_3DLUT_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_3DLUT_PARAMS));
            DDI_CHK_NULL(pSrc->p3DLutParams, "p3DLutParams MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        pSrc->p3DLutParams->LutSize               = (uint32_t)p3DLutParamBuff->lut_size;
        pSrc->p3DLutParams->BitDepthPerChannel    = (uint16_t)p3DLutParamBuff->bit_depth;
        pSrc->p3DLutParams->ByteCountPerEntry     = (uint16_t)(p3DLutParamBuff->num_channel * (pSrc->p3DLutParams->BitDepthPerChannel / 8));
        if (nullptr == pSrc->p3DLutParams->pExt3DLutSurface)
        {
            pSrc->p3DLutParams->pExt3DLutSurface      = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
            DDI_CHK_NULL(pSrc->p3DLutParams->pExt3DLutSurface, "p3DLutParams MOS_AllocAndZeroMemory failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        if (pSrc->p3DLutParams->pExt3DLutSurface)
        {
            PMOS_RESOURCE pOsResource     = nullptr;
            pOsResource                   = &(pSrc->p3DLutParams->pExt3DLutSurface->OsResource);
            DDI_CHK_NULL(pOsResource, "Null  pOsResource.", VA_STATUS_ERROR_INVALID_PARAMETER);

            DdiMedia_MediaSurfaceToMosResource(pMedia3DLutSurf, pOsResource);
            Mos_Solo_SetOsResource(pMedia3DLutSurf->pGmmResourceInfo, pOsResource);
        }
    }

    return eStatus;
}
#endif
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
    void                       *pCtx,
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
    DDI_CHK_NULL(pCtx, "Null pCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pVaDrvCtx,
                    "Null pVaDrvCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    vaStatus    = VA_STATUS_SUCCESS;
    *pVaBufID   = VA_INVALID_ID;

    pMediaCtx   = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL(pMediaCtx,
                    "Null pMediaCtx.",
                    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_VP_CONTEXT  pVpCtx = (PDDI_VP_CONTEXT)pCtx;

    // only for VAProcFilterParameterBufferType and VAProcPipelineParameterBufferType
    if (vaBufType != VAProcFilterParameterBufferType
        && vaBufType != VAProcPipelineParameterBufferType
#if VA_CHECK_VERSION(1, 10, 0)
        && vaBufType != VAContextParameterUpdateBufferType
#endif
       )
    {
        VP_DDI_ASSERTMESSAGE("Unsupported Va Buffer Type.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // allocate new buf and init
    pBuf               = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    DDI_CHK_NULL(pBuf, "Null pBuf.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    pBuf->pMediaCtx     = pMediaCtx;
    pBuf->iSize         = uiSize * uiNumElements;
    pBuf->uiNumElements = uiNumElements;
    pBuf->uiType        = vaBufType;
    pBuf->format        = Media_Format_Buffer;
    pBuf->uiOffset      = 0;
    pBuf->pData         = (uint8_t*)MOS_AllocAndZeroMemory(uiSize * uiNumElements);
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
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

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
    pVpCtx = MOS_New(DDI_VP_CONTEXT);
    DDI_CHK_NULL(pVpCtx, "Null pVpCtx.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // init pVpCtx
    vaStatus = DdiVp_InitCtx(pVaDrvCtx, pVpCtx);
    DDI_CHK_RET(vaStatus, "VA_STATUS_ERROR_OPERATION_FAILED");

    DdiMediaUtil_LockMutex(&pMediaCtx->VpMutex);

    // get Free VP context index
    pVaCtxHeapElmt = DdiMediaUtil_AllocPVAContextFromHeap(pMediaCtx->pVpCtxHeap);
    if (nullptr == pVaCtxHeapElmt)
    {
        MOS_Delete(pVpCtx);
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
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

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
        Delete_DdiCpInterface(pVpCtx->pCpDdiInterface);
        pVpCtx->pCpDdiInterface = NULL;
    }

    // destroy vphal
    vaStatus  = DdiVp_DestroyVpHal(pVpCtx);

    // Get VP context index
    uiVpIndex = vaCtxID & DDI_MEDIA_MASK_VACONTEXTID;

    // remove from context array
    DdiMediaUtil_LockMutex(&pMediaCtx->VpMutex);
    // destroy vp context
    MOS_Delete(pVpCtx);
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
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

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

    // Set os resource for VPHal render
    vaStatus = VpSetOsResource(pVpCtx, pMediaTgtSurf, pVpHalRenderParams->uDstCount);
    DDI_CHK_RET(vaStatus, "Call VpSetOsResource failed");

    pVpHalTgtSurf->Format   = pVpHalTgtSurf->OsResource.Format;
    pVpHalTgtSurf->TileType = pVpHalTgtSurf->OsResource.TileType;

    // reset source surface count
    pVpHalRenderParams->uSrcCount = 0;

    pVpHalRenderParams->bReportStatus    = true;
    pVpHalRenderParams->StatusFeedBackID = vaSurfID;

    if (pMediaTgtSurf->pSurfDesc && (pMediaTgtSurf->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR))
    {
        pVpHalRenderParams->pTarget[pVpHalRenderParams->uDstCount]->b16UsrPtr = VpIs16UsrPtrPitch(pMediaTgtSurf->iPitch, pMediaTgtSurf->format);
    }
    else
    {
        pVpHalRenderParams->pTarget[pVpHalRenderParams->uDstCount]->b16UsrPtr = false;
    }

    // increase render target count
    pVpHalRenderParams->uDstCount++;

    return VA_STATUS_SUCCESS;
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
VAStatus DdiVp_BeginPictureInt(
        VADriverContextP    pVaDrvCtx,
        PDDI_VP_CONTEXT     pVpCtx,
        VASurfaceID         vaSurfID)
{
    PDDI_MEDIA_CONTEXT          pMediaDrvCtx;
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

    // Set os resource for VPHal render
    vaStatus = VpSetOsResource(pVpCtx, pMediaTgtSurf, pVpHalRenderParams->uDstCount);
    DDI_CHK_RET(vaStatus, "Call VpSetOsResource failed");

    pVpHalTgtSurf->Format = pVpHalTgtSurf->OsResource.Format;

    pVpHalRenderParams->bReportStatus    = true;
    pVpHalRenderParams->StatusFeedBackID = vaSurfID;
    if (pMediaTgtSurf->pSurfDesc && (pMediaTgtSurf->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR))
    {
        pVpHalRenderParams->pTarget[pVpHalRenderParams->uDstCount]->b16UsrPtr = VpIs16UsrPtrPitch(pMediaTgtSurf->iPitch, pMediaTgtSurf->format);
    }
    else
    {
        pVpHalRenderParams->pTarget[pVpHalRenderParams->uDstCount]->b16UsrPtr = false;
    }
    // increase render target count
    pVpHalRenderParams->uDstCount++;

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
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    PDDI_MEDIA_CONTEXT        pMediaCtx;
    PDDI_VP_CONTEXT           pVpCtx;
    PDDI_MEDIA_BUFFER         pBuf;
    int32_t                   i;
    void                      *pData;
    uint32_t                  ctxType;
    VAStatus                  vaStatus;
    int32_t                   numOfBuffers              = num_buffers;
    int32_t                   priority                  = 0;
    int32_t                   priorityIndexInBuffers    = -1;
    bool                      updatePriority            = false;

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
    DDI_CHK_CONDITION(((numOfBuffers > VPHAL_MAX_SOURCES) || (numOfBuffers <= 0)),
                         "numOfBuffers is Invalid.",
                         VA_STATUS_ERROR_INVALID_PARAMETER);
    
    priorityIndexInBuffers = DdiMedia_GetGpuPriority(pVaDrvCtx, buffers, numOfBuffers, &updatePriority, &priority);
    if (priorityIndexInBuffers != -1)
    {
        if(updatePriority)
        {
            vaStatus = DdiVp_SetGpuPriority(pVpCtx, priority);
            if(vaStatus != VA_STATUS_SUCCESS)
                return vaStatus;
        }
        MovePriorityBufferIdToEnd(buffers, priorityIndexInBuffers, numOfBuffers);
        numOfBuffers--;
    }
    if (numOfBuffers == 0)
        return vaStatus;

    for (i = 0; i < numOfBuffers; i++)
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
                vaStatus = VA_STATUS_ERROR_INVALID_BUFFER;
                break;

            default:
                DDI_CHK_RET(vaStatus,"Unsupported buffer type!");
                break;
        }
        DdiMedia_UnmapBuffer(pVaDrvCtx, buffers[i]);
    }
    return vaStatus;
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
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    PDDI_VP_CONTEXT         pVpCtx;
    uint32_t                uiCtxType;
    VpBase                  *pVpHal;
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
    pVpCtx->pVpHalRenderParams->Component = COMPONENT_VPCommon;

    pVpHal  = pVpCtx->pVpHal;
    DDI_CHK_NULL(pVpHal, "Null pVpHal.", VA_STATUS_ERROR_INVALID_PARAMETER);
    eStatus = pVpHal->Render(pVpCtx->pVpHalRenderParams);

#if (_DEBUG || _RELEASE_INTERNAL)
    VpDumpProcPipelineParams(pVaDrvCtx, pVpCtx);
#endif //(_DEBUG || _RELEASE_INTERNAL)

    VpReportFeatureMode(pVpCtx);

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
//! \purpose
//!  Execute video processing pipeline.
//!  For CSC/Scaling after decode by decode context.
//! \params
//! [in]  pVaDrvCtxv    : VA Driver Context
//! [in]  vaCtxID       : VA Context ID
//! [in]  srcSurface    : Input surface ID
//! [in]  srcRect       : Rectangle of the input surface region
//! [in]  dstSurface    : Output surface ID
//! [in]  dstRect       : Rectangle of the output surface region
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus DdiVp_VideoProcessPipeline(
    VADriverContextP    pVaDrvCtx,
    VAContextID         vpCtxID,
    VASurfaceID         srcSurface,
    VARectangle         *srcRect,
    VASurfaceID         dstSurface,
    VARectangle         *dstRect)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    VAStatus            vaStatus;
    uint32_t            ctxType;
    PDDI_VP_CONTEXT     pVpCtx;

    VP_DDI_FUNCTION_ENTER;
    vaStatus = VA_STATUS_SUCCESS;
    DDI_CHK_NULL(pVaDrvCtx, "nullptr pVaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pVpCtx = (PDDI_VP_CONTEXT)DdiMedia_GetContextFromContextID(pVaDrvCtx, vpCtxID, &ctxType);
    DDI_CHK_NULL(pVpCtx, "nullptr pVpCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    vaStatus = DdiVp_BeginPicture(pVaDrvCtx, vpCtxID, dstSurface);
    DDI_CHK_RET(vaStatus, "VP BeginPicture failed");

    //Set parameters
    VAProcPipelineParameterBuffer* pInputPipelineParam = (VAProcPipelineParameterBuffer*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineParameterBuffer));
    DDI_CHK_NULL(pInputPipelineParam, "nullptr pInputPipelineParam", VA_STATUS_ERROR_ALLOCATION_FAILED);

    pInputPipelineParam->surface_region = srcRect;
    pInputPipelineParam->output_region  = dstRect;
    pInputPipelineParam->surface        = srcSurface;

    vaStatus = DdiVp_SetProcPipelineParams(pVaDrvCtx, pVpCtx, pInputPipelineParam);
    if(vaStatus != VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(pInputPipelineParam);
        DDI_ASSERTMESSAGE("VP SetProcPipelineParams failed.");
        return vaStatus;
    }

    vaStatus = DdiVp_EndPicture(pVaDrvCtx, vpCtxID);
    if(vaStatus != VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(pInputPipelineParam);
        DDI_ASSERTMESSAGE("VP EndPicture failed.");
        return vaStatus;
    }

    MOS_FreeMemory(pInputPipelineParam);
    return vaStatus;
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
            case Format_Y410:
            case Format_Y416:
            case Format_R10G10B10A2:
            case Format_B10G10R10A2:
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
    PVPHAL_SURFACE            pTarget;
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
    pTarget = pVpHalRenderParams->pTarget[0];
    DDI_CHK_NULL(pTarget, "Null pTarget.", VA_STATUS_ERROR_INVALID_SURFACE);

    //For surface with alpha, we need to bypass SFC that could change the output alpha.
    if (hasAlphaInSurface(pSrc) &&
        hasAlphaInSurface(pTarget))
    {
        pVpHalRenderParams->pCompAlpha->fAlpha      = 0.0f;
        pVpHalRenderParams->pCompAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM;
    }
    else
    {
        pVpHalRenderParams->pCompAlpha->fAlpha      = 1.0f;
        pVpHalRenderParams->pCompAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_NONE;
    }

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
//! \purpose Update the future reference frames for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpHalSrcSurf : VpHal source surface
//! [in]  pPipelineParam : Pipeline parameter from application (VAProcPipelineParameterBuffer)
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_UpdateProcPipelineFutureReferenceFrames(
    PDDI_VP_CONTEXT                 pVpCtx,
    VADriverContextP                pVaDrvCtx,
    PVPHAL_SURFACE                  pVpHalSrcSurf,
    VAProcPipelineParameterBuffer*  pPipelineParam)
{
    PVPHAL_RENDER_PARAMS pVpHalRenderParams;
    PVPHAL_SURFACE       pSurface;
    uint32_t i;

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

    // DDI regard backward_references as future frame, but VPHAL regard pFwdRef as future frame
    if (pPipelineParam->backward_references != nullptr)
    {
        for (i = 0;i < pPipelineParam->num_backward_references; i++)
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
                pSurface->uFwdRefCount           = pPipelineParam->num_backward_references - i;
            }
            pRefSurfBuffObj = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, pPipelineParam->backward_references[i]);
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
//! \purpose Update the past reference frames for VPHAL input surface
//! \params
//! [in]  pVpCtx : VP context
//! [in]  pVaDrvCtx : VA Driver context
//! [in]  pVpHalSrcSurf : VpHal source surface
//! [in]  pPipelineParam : Pipeline parameter from application (VAProcPipelineParameterBuffer)
//! [out] None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiVp_UpdateProcPipelinePastReferenceFrames(
    PDDI_VP_CONTEXT                 pVpCtx,
    VADriverContextP                pVaDrvCtx,
    PVPHAL_SURFACE                  pVpHalSrcSurf,
    VAProcPipelineParameterBuffer*  pPipelineParam)
{
    PVPHAL_RENDER_PARAMS pVpHalRenderParams;
    PVPHAL_SURFACE       pSurface;
    uint32_t i;

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

    // DDI regard forward_references as past frame, but VPHAL regard pBwdRef as past frame
    if (pPipelineParam->forward_references  != nullptr)
    {
        for (i = 0;i < pPipelineParam->num_forward_references ; i++)
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
                pSurface->uBwdRefCount           = pPipelineParam->num_forward_references  - i;;
            }
            pRefSurfBuffObj = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, pPipelineParam->forward_references [i]);
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

    uint32_t uCnt            = 0;   /* used for loop */
    uint32_t uQueryCapsNum   = 0;   /* the filter caps number queried by app layer */
    uint32_t uExistCapsNum   = 0;   /* the actual number of filters in vp module */
    uint32_t uQueryFlag      = 0;   /* QUERY_CAPS_ATTRIBUTE: search caps attribute */
    PDDI_MEDIA_CONTEXT mediaDrvCtx = nullptr;

    VP_DDI_FUNCTION_ENTER;

    DDI_CHK_NULL (num_filter_caps, "Null num_filter_caps.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL (pVaDrvCtx, "Null pVaDrvCtx.", VA_STATUS_ERROR_INVALID_PARAMETER);

    mediaDrvCtx = DdiMedia_GetMediaContext(pVaDrvCtx);
    DDI_CHK_NULL (mediaDrvCtx, "Null mediaDrvCtx.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (*num_filter_caps != 0)
    {
        /* return filter caps attribute  */
        uQueryFlag = QUERY_CAPS_ATTRIBUTE;
        /* check filter_caps pointer to fill in the filter_caps */
        DDI_CHK_NULL (filter_caps, "Null filter_caps.", VA_STATUS_ERROR_INVALID_PARAMETER);
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

                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }

                baseCap->range.min_value     = NOISEREDUCTION_MIN;
                baseCap->range.max_value     = NOISEREDUCTION_MAX;
                baseCap->range.default_value = NOISEREDUCTION_DEFAULT;
                baseCap->range.step          = NOISEREDUCTION_STEP;
            }
            break;
        
        /* HVS Noise reduction filter */
        case VAProcFilterHVSNoiseReduction:
            if (mediaDrvCtx)
            {
                if (MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrHVSDenoise))
                {
                    uExistCapsNum = 4;
                    *num_filter_caps = uExistCapsNum;
                }
                else
                {
                    uExistCapsNum = 0;
                    *num_filter_caps = uExistCapsNum;
                }
            }
            else
            {
                VP_DDI_ASSERTMESSAGE("mediaDrvCtx is null pointer.\n");
                return VA_STATUS_ERROR_INVALID_CONTEXT;
            }
            break;

        /* Deinterlacing filter */
        case VAProcFilterDeinterlacing:
            uExistCapsNum = 3;
            /* set input filter caps number to the actual number of filters in vp module */
            *num_filter_caps = uExistCapsNum;
            /* set the actual filter caps attribute in vp module */
            if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
            {
                VAProcFilterCapDeinterlacing* diCap = (VAProcFilterCapDeinterlacing*) filter_caps;

                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }

                diCap[0].type = VAProcDeinterlacingBob;
                diCap[1].type = VAProcDeinterlacingMotionAdaptive;
                diCap[2].type = VAProcDeinterlacingMotionCompensated;
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

                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }

                baseCap                      = (VAProcFilterCap*) filter_caps;
                baseCap->range.min_value     = EDGEENHANCEMENT_MIN;
                baseCap->range.max_value     = EDGEENHANCEMENT_MAX;
                baseCap->range.default_value = EDGEENHANCEMENT_DEFAULT;
                baseCap->range.step          = EDGEENHANCEMENT_STEP;
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

                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }

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
            }
            break;
        case VAProcFilterSkinToneEnhancement:
            uExistCapsNum  = 1;
            *num_filter_caps = uExistCapsNum;
            if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
            {
                VAProcFilterCap* baseCap     = (VAProcFilterCap *)filter_caps;

                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }

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

                if (uQueryCapsNum < uExistCapsNum)
                {
                    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                }

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
            }
            break;
        case VAProcFilterHighDynamicRangeToneMapping:
        {
            if (mediaDrvCtx)
            {
                if (MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrHDR))
                {
                    uExistCapsNum = 1;
                    *num_filter_caps = uExistCapsNum;
                    if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
                    {
                        VAProcFilterCapHighDynamicRange *HdrTmCap = (VAProcFilterCapHighDynamicRange *)filter_caps;

                        if (uQueryCapsNum < uExistCapsNum)
                        {
                            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                        }

                        if (HdrTmCap)
                        {
                            HdrTmCap->metadata_type = VAProcHighDynamicRangeMetadataHDR10;
                            HdrTmCap->caps_flag = VA_TONE_MAPPING_HDR_TO_HDR | VA_TONE_MAPPING_HDR_TO_SDR | VA_TONE_MAPPING_HDR_TO_EDR;
                        }
                    }
                }
                else
                {
                    uExistCapsNum = 0;
                    *num_filter_caps = uExistCapsNum;
                }
            }
            else
            {
                VP_DDI_ASSERTMESSAGE("mediaDrvCtx is null pointer.\n");
                return VA_STATUS_ERROR_INVALID_CONTEXT;
            }
            break;
        }
#if VA_CHECK_VERSION(1, 12, 0)
        case VAProcFilter3DLUT:
            uExistCapsNum = 0;
            *num_filter_caps = uExistCapsNum;
            /* 3DLUT is supported in VEBOX on Gen11+*/
            if (!MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrDisableVEBoxFeatures))
            {
                /* 3DLUT exposes 3 kinds of caps on Gen11+*/
                if (mediaDrvCtx->platform.eRenderCoreFamily >= IGFX_GEN12_CORE)
                {
                    uExistCapsNum = 3;
                    *num_filter_caps = uExistCapsNum;
                    /* set the actual filter caps attribute in vp module */
                    if (uQueryFlag == QUERY_CAPS_ATTRIBUTE)
                    {
                        if (uQueryCapsNum < uExistCapsNum)
                        {
                            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
                        }
                        uint32_t channel_mapping = VA_3DLUT_CHANNEL_RGB_RGB |
                                                   VA_3DLUT_CHANNEL_YUV_RGB |
                                                   VA_3DLUT_CHANNEL_VUY_RGB;

                        VAProcFilterCap3DLUT* lut3DCap = nullptr;
                        /* 17^3 */
                        lut3DCap                      = (VAProcFilterCap3DLUT *)filter_caps + 0;
                        lut3DCap->lut_size            = 17;
                        lut3DCap->lut_stride[0]       = 17;
                        lut3DCap->lut_stride[1]       = 17;
                        lut3DCap->lut_stride[2]       = 32;
                        lut3DCap->bit_depth           = 16;
                        lut3DCap->num_channel         = 4;
                        lut3DCap->channel_mapping     = channel_mapping;
                        /* 33^3 */
                        lut3DCap                      = (VAProcFilterCap3DLUT *)filter_caps + 1;
                        lut3DCap->lut_size            = 33;
                        lut3DCap->lut_stride[0]       = 33;
                        lut3DCap->lut_stride[1]       = 33;
                        lut3DCap->lut_stride[2]       = 64;
                        lut3DCap->bit_depth           = 16;
                        lut3DCap->num_channel         = 4;
                        lut3DCap->channel_mapping     = channel_mapping;
                        /* 65^3 */
                        lut3DCap                      = (VAProcFilterCap3DLUT *)filter_caps + 2;
                        lut3DCap->lut_size            = 65;
                        lut3DCap->lut_stride[0]       = 65;
                        lut3DCap->lut_stride[1]       = 65;
                        lut3DCap->lut_stride[2]       = 128;
                        lut3DCap->bit_depth           = 16;
                        lut3DCap->num_channel         = 4;
                        lut3DCap->channel_mapping     = channel_mapping;
                    }
                }
            }
            break;
#endif
        case VAProcFilterCount:
        case VAProcFilterNone:
            return VA_STATUS_ERROR_INVALID_VALUE;

        default:
            return VA_STATUS_ERROR_UNSUPPORTED_FILTER;

    }

    return VA_STATUS_SUCCESS;
}// DdiVp_QueryVideoProcFilterCaps()

VAStatus DdiVp_SetGpuPriority(
    PDDI_VP_CONTEXT     pVpCtx,
    int32_t             priority
)
{
    DDI_CHK_NULL(pVpCtx, "nullptr pVpCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    //Set the priority for Gpu
    if(pVpCtx->pVpHal != nullptr)
    {
        PMOS_INTERFACE osInterface = pVpCtx->pVpHal->GetOsInterface();
        DDI_CHK_NULL(osInterface, "nullptr osInterface.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        osInterface->pfnSetGpuPriority(osInterface, priority);
    }

    return VA_STATUS_SUCCESS;
}

