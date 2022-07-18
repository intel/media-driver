/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_render_common.c
//! \brief    The file of common utilities definitions shared by low level renderers
//! \details  Common utilities for different renderers, e.g. DNDI or Comp
//!
#include "media_render_common.h"

//!
//! \brief    Initialized RenderHal Surface Type
//! \details  Initialized RenderHal Surface Type according to input VPHAL Surface Type
//! \param    [in] vpSurfType
//!           VPHAL surface type
//! \return   RENDERHAL_SURFACE_TYPE
//!
static inline RENDERHAL_SURFACE_TYPE RndrInitRenderHalSurfType(VPHAL_SURFACE_TYPE vpSurfType)
{
    switch (vpSurfType)
    {
        case SURF_IN_BACKGROUND:
            return RENDERHAL_SURF_IN_BACKGROUND;

        case SURF_IN_PRIMARY:
            return RENDERHAL_SURF_IN_PRIMARY;

        case SURF_IN_SUBSTREAM:
            return RENDERHAL_SURF_IN_SUBSTREAM;

        case SURF_IN_REFERENCE:
            return RENDERHAL_SURF_IN_REFERENCE;

        case SURF_OUT_RENDERTARGET:
            return RENDERHAL_SURF_OUT_RENDERTARGET;

        case SURF_NONE:
        default:
            return RENDERHAL_SURF_NONE;
    }
}

//!
//! \brief    Initialized RenderHal Scaling Mode
//! \details  Initialized RenderHal Scaling Mode according to input VPHAL Scaling Mode
//! \param    [in] vpScalingMode
//!           VPHAL Scaling Mode
//! \return   RENDERHAL_SCALING_MODE
//!
static inline RENDERHAL_SCALING_MODE RndrInitRenderHalScalingMode(VPHAL_SCALING_MODE vpScalingMode)
{
    switch (vpScalingMode)
    {
        case VPHAL_SCALING_NEAREST:
            return RENDERHAL_SCALING_NEAREST;

        case VPHAL_SCALING_BILINEAR:
            return RENDERHAL_SCALING_BILINEAR;

        case VPHAL_SCALING_AVS:
            return RENDERHAL_SCALING_AVS;

        default:
            VP_RENDER_ASSERTMESSAGE("Invalid VPHAL_SCALING_MODE %d, force to nearest mode.", vpScalingMode);
            return RENDERHAL_SCALING_NEAREST;
    }
}

//!
//! \brief    Get VpHal Scaling Mode
//! \details  Get VpHal Scaling Mode according to RenderHal Scaling Mode
//! \param    [in] RenderHalScalingMode
//!           RENDERHAL Scaling Mode
//! \return   VPHAL_SCALING_MODE
//!
static inline VPHAL_SCALING_MODE RndrGetVpHalScalingMode(RENDERHAL_SCALING_MODE RenderHalScalingMode)
{
    switch (RenderHalScalingMode)
    {
        case RENDERHAL_SCALING_NEAREST:
            return VPHAL_SCALING_NEAREST;

        case RENDERHAL_SCALING_BILINEAR:
            return VPHAL_SCALING_BILINEAR;

        case RENDERHAL_SCALING_AVS:
            return VPHAL_SCALING_AVS;

        default:
            VP_RENDER_ASSERTMESSAGE("Invalid RENDERHAL_SCALING_MODE %d, force to nearest mode.", RenderHalScalingMode);
            return VPHAL_SCALING_NEAREST;
    }
}

//!
//! \brief    Initialized RenderHal Sample Type
//! \details  Initialized RenderHal Sample Type according to input VPHAL Sample Type
//! \param    [in] SampleType
//!           VPHAL Sample Type
//! \return   RENDERHAL_SAMPLE_TYPE
//!
static inline RENDERHAL_SAMPLE_TYPE RndrInitRenderHalSampleType(VPHAL_SAMPLE_TYPE SampleType)
{
    switch (SampleType)
    {
        case SAMPLE_PROGRESSIVE:
            return RENDERHAL_SAMPLE_PROGRESSIVE;

        case SAMPLE_SINGLE_TOP_FIELD:
            return RENDERHAL_SAMPLE_SINGLE_TOP_FIELD;

        case SAMPLE_SINGLE_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_SINGLE_BOTTOM_FIELD;

        case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;

        case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;

        case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD;

        case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;

        case SAMPLE_INVALID:
        default:
            VP_RENDER_ASSERTMESSAGE("Invalid VPHAL_SAMPLE_TYPE %d.\n", SampleType);
            return RENDERHAL_SAMPLE_INVALID;
    }
}

static inline void RndrInitPlaneOffset(
    PMOS_PLANE_OFFSET pMosPlaneOffset,
    PVPHAL_PLANE_OFFSET pVpPlaneOffset)
{
    pMosPlaneOffset->iSurfaceOffset     = pVpPlaneOffset->iSurfaceOffset;
    pMosPlaneOffset->iXOffset           = pVpPlaneOffset->iXOffset;
    pMosPlaneOffset->iYOffset           = pVpPlaneOffset->iYOffset;
    pMosPlaneOffset->iLockSurfaceOffset = pVpPlaneOffset->iLockSurfaceOffset;

}

//!
//! \brief    Initialized MHW Rotation mode
//! \details  Initialized MHW Rotation mode according to input VPHAL Rotation Type
//! \param    [in] Rotation
//!           VPHAL Rotation mode
//! \return   MHW_ROTATION
//!
static inline MHW_ROTATION RndrInitRotationMode(VPHAL_ROTATION Rotation)
{
    MHW_ROTATION    Mode = MHW_ROTATION_IDENTITY;

    switch (Rotation)
    {
        case VPHAL_ROTATION_IDENTITY:
            Mode = MHW_ROTATION_IDENTITY;
            break;

        case VPHAL_ROTATION_90:
            Mode = MHW_ROTATION_90;
            break;

        case VPHAL_ROTATION_180:
            Mode = MHW_ROTATION_180;
            break;

        case VPHAL_ROTATION_270:
            Mode = MHW_ROTATION_270;
            break;

        case VPHAL_MIRROR_HORIZONTAL:
            Mode = MHW_MIRROR_HORIZONTAL;
            break;

        case VPHAL_MIRROR_VERTICAL:
            Mode = MHW_MIRROR_VERTICAL;
            break;

        case VPHAL_ROTATE_90_MIRROR_VERTICAL:
            Mode = MHW_ROTATE_90_MIRROR_VERTICAL;
            break;

        case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
            Mode = MHW_ROTATE_90_MIRROR_HORIZONTAL;
            break;

        default:
            VP_RENDER_ASSERTMESSAGE("Invalid Rotation Angle.");
            break;
    }

    return Mode;
}

//!
//! \brief    Initialized RenderHal Surface according to incoming VPHAL Surface
//! \param    [in] pVpSurface
//!           Pointer to the VPHAL surface
//! \param    [out] pRenderHalSurface
//!           Pointer to the RenderHal surface
//! \return   MOS_STATUS
//!
MOS_STATUS MediaRenderCommon::RndrCommonInitRenderHalSurface(
    PVPHAL_SURFACE          pVpSurface,
    PRENDERHAL_SURFACE      pRenderHalSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------
    VP_RENDER_CHK_NULL(pVpSurface);
    VP_RENDER_CHK_NULL(pRenderHalSurface);
    //---------------------------------------

    MOS_ZeroMemory(pRenderHalSurface, sizeof(*pRenderHalSurface));

    pRenderHalSurface->OsSurface.OsResource         = pVpSurface->OsResource;
    pRenderHalSurface->OsSurface.dwWidth            = pVpSurface->dwWidth;
    pRenderHalSurface->OsSurface.dwHeight           = pVpSurface->dwHeight;
    pRenderHalSurface->OsSurface.dwPitch            = pVpSurface->dwPitch;
    pRenderHalSurface->OsSurface.Format             = pVpSurface->Format;
    pRenderHalSurface->OsSurface.TileType           = pVpSurface->TileType;
    pRenderHalSurface->OsSurface.TileModeGMM        = pVpSurface->TileModeGMM;
    pRenderHalSurface->OsSurface.bGMMTileEnabled    = pVpSurface->bGMMTileEnabled;
    pRenderHalSurface->OsSurface.dwOffset           = pVpSurface->dwOffset;
    pRenderHalSurface->OsSurface.bIsCompressed      = pVpSurface->bIsCompressed;
    pRenderHalSurface->OsSurface.bCompressible      = pVpSurface->bCompressible;
    pRenderHalSurface->OsSurface.CompressionMode    = pVpSurface->CompressionMode;
    pRenderHalSurface->OsSurface.dwDepth            = pVpSurface->dwDepth;
    pRenderHalSurface->OsSurface.dwQPitch           = pVpSurface->dwHeight;
    pRenderHalSurface->OsSurface.MmcState           = (MOS_MEMCOMP_STATE)pVpSurface->CompressionMode;
    pRenderHalSurface->OsSurface.CompressionFormat  = pVpSurface->CompressionFormat;

    RndrInitPlaneOffset(
        &pRenderHalSurface->OsSurface.YPlaneOffset,
        &pVpSurface->YPlaneOffset);
    RndrInitPlaneOffset(
        &pRenderHalSurface->OsSurface.UPlaneOffset,
        &pVpSurface->UPlaneOffset);
    RndrInitPlaneOffset(
        &pRenderHalSurface->OsSurface.VPlaneOffset,
        &pVpSurface->VPlaneOffset);

    pRenderHalSurface->rcSrc                        = pVpSurface->rcSrc;
    pRenderHalSurface->rcDst                        = pVpSurface->rcDst;
    pRenderHalSurface->rcMaxSrc                     = pVpSurface->rcMaxSrc;
    pRenderHalSurface->SurfType                     =
                    RndrInitRenderHalSurfType(pVpSurface->SurfType);
    pRenderHalSurface->ScalingMode                  =
                    RndrInitRenderHalScalingMode(pVpSurface->ScalingMode);
    pRenderHalSurface->ChromaSiting                 = pVpSurface->ChromaSiting;

    if (pVpSurface->pDeinterlaceParams != nullptr)
    {
        pRenderHalSurface->bDeinterlaceEnable       = true;
    }
    else
    {
        pRenderHalSurface->bDeinterlaceEnable       = false;
    }

    pRenderHalSurface->iPaletteID                   = pVpSurface->iPalette;

    pRenderHalSurface->bQueryVariance               = pVpSurface->bQueryVariance;
    pRenderHalSurface->bInterlacedScaling           = pVpSurface->bInterlacedScaling;
    pRenderHalSurface->pDeinterlaceParams           = (void *)pVpSurface->pDeinterlaceParams;
    pRenderHalSurface->SampleType                   =
                    RndrInitRenderHalSampleType(pVpSurface->SampleType);

    pRenderHalSurface->Rotation                     =
                    RndrInitRotationMode(pVpSurface->Rotation);

finish:
    return eStatus;
}

//!
//! \brief    Get output RenderHal Surface parameters back to VPHAL Surface
//! \param    [in] pRenderHalSurface
//!           Pointer to the RenderHal surface
//! \param    [in,out] pVpSurface
//!           Pointer to the VPHAL surface
//! \return   MOS_STATUS
//!
MOS_STATUS MediaRenderCommon::RndrCommonGetBackVpSurfaceParams(
    PRENDERHAL_SURFACE      pRenderHalSurface,
    PVPHAL_SURFACE          pVpSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------
    VP_RENDER_CHK_NULL(pVpSurface);
    VP_RENDER_CHK_NULL(pRenderHalSurface);
    //---------------------------------------

    // The following params are mostly for b32MWColorFillKern on Gen75/Gen8 only
    pVpSurface->dwHeight                            = pRenderHalSurface->OsSurface.dwHeight;
    pVpSurface->dwPitch                             = pRenderHalSurface->OsSurface.dwPitch;
    pVpSurface->Format                              = pRenderHalSurface->OsSurface.Format;
    pVpSurface->dwOffset                            = pRenderHalSurface->OsSurface.dwOffset;
    pVpSurface->YPlaneOffset.iXOffset               = pRenderHalSurface->OsSurface.YPlaneOffset.iXOffset;
    pVpSurface->YPlaneOffset.iYOffset               = pRenderHalSurface->OsSurface.YPlaneOffset.iYOffset;
    pVpSurface->UPlaneOffset.iSurfaceOffset         = pRenderHalSurface->OsSurface.UPlaneOffset.iSurfaceOffset;
    pVpSurface->VPlaneOffset.iSurfaceOffset         = pRenderHalSurface->OsSurface.VPlaneOffset.iSurfaceOffset;
    pVpSurface->rcDst                               = pRenderHalSurface->rcDst;

    pVpSurface->dwWidth                             = pRenderHalSurface->OsSurface.dwWidth;
    pVpSurface->ScalingMode                         = RndrGetVpHalScalingMode(pRenderHalSurface->ScalingMode);

finish:
    return eStatus;
}

//!
//! \brief    Set Surface for HW Access for CP HM
//! \details  Common Function for setting up surface state, need to use this function
//!           if render would use CP HM 
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pRenderSurface
//!           Pointer to Render Surface
//! \param    [in] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to bind surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS MediaRenderCommon::CommonSetSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE                  pRenderSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite)
{

    PMOS_INTERFACE                  pOsInterface;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                         iSurfaceEntries;
    int32_t                         i;
    MOS_STATUS                      eStatus;

    // Initialize Variables
    eStatus                         = MOS_STATUS_SUCCESS;
    pOsInterface                    = pRenderHal->pOsInterface;

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    VP_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pSurface->OsResource,
        bWrite,
        true));

    VP_RENDER_CHK_STATUS(RndrCommonInitRenderHalSurface(
        pSurface,
        pRenderSurface));

    // Setup surface states-----------------------------------------------------
    VP_RENDER_CHK_STATUS(pRenderHal->pfnSetupSurfaceState(
        pRenderHal,
        pRenderSurface,
        pSurfaceParams,
        &iSurfaceEntries,
        pSurfaceEntries,
        nullptr));

    VP_RENDER_CHK_STATUS(RndrCommonGetBackVpSurfaceParams(
        pRenderSurface,
        pSurface));

    // Bind surface states------------------------------------------------------
    for (i = 0; i < iSurfaceEntries; i++, iBTEntry++)
    {
        VP_RENDER_CHK_STATUS(pRenderHal->pfnBindSurfaceState(
            pRenderHal,
            iBindingTable,
            iBTEntry,
            pSurfaceEntries[i]));
    }

finish:
    return eStatus;
}

//!
//! \brief    Set Buffer Surface for HW Access for CP HM
//! \details  Common Function for setting up buffer surface state, need to use this function
//!           if render would use CP HM
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pRenderSurface
//!           Pointer to Render Surface
//! \param    [in,out] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to Bind Surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS MediaRenderCommon::CommonSetBufferSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE                  pRenderSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite)
{
    PMOS_INTERFACE                      pOsInterface;
    RENDERHAL_SURFACE_STATE_PARAMS      SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY      pSurfaceEntry;
    MOS_STATUS                          eStatus;

    VP_RENDER_CHK_NULL(pRenderHal);
    VP_RENDER_CHK_NULL(pRenderHal->pOsInterface);

    // Initialize Variables
    eStatus                         = MOS_STATUS_SUCCESS;
    pOsInterface                    = pRenderHal->pOsInterface;

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    VP_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pSurface->OsResource,
        bWrite,
        true));

    // Setup Buffer surface-----------------------------------------------------
    if (pSurfaceParams == nullptr)
    {
        MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));

        //set mem object control for cache
        SurfaceParam.MemObjCtl = (pRenderHal->pOsInterface->pfnCachePolicyGetMemoryObject(
            MOS_MP_RESOURCE_USAGE_DEFAULT,
            pRenderHal->pOsInterface->pfnGetGmmClientContext(pRenderHal->pOsInterface))).DwordValue;

        pSurfaceParams = &SurfaceParam;
    }

    VP_RENDER_CHK_STATUS(RndrCommonInitRenderHalSurface(
        pSurface,
        pRenderSurface));

    VP_RENDER_CHK_STATUS(pRenderHal->pfnSetupBufferSurfaceState(
        pRenderHal,
        pRenderSurface,
        pSurfaceParams,
        &pSurfaceEntry));

    // Bind surface state-------------------------------------------------------
    VP_RENDER_CHK_STATUS(pRenderHal->pfnBindSurfaceState(
        pRenderHal,
        iBindingTable,
        iBTEntry,
        pSurfaceEntry));

finish:
    return eStatus;
}
