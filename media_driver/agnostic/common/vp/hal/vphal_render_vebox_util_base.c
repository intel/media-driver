/*
* Copyright (c) 2014-2018, Intel Corporation
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
//! \file     vphal_render_vebox_util_base.c
//! \brief    Common utility interface used in Vebox
//! \details  Common utility interface used in Vebox which are platform independent
//!
#include "vphal_render_vebox_util_base.h"

MHW_CSPACE VPHal_VpHalCspace2MhwCspace(VPHAL_CSPACE cspace)
{
    switch (cspace)
    {
        case CSpace_Source:
            return MHW_CSpace_Source;

        case CSpace_RGB:
            return MHW_CSpace_RGB;

        case CSpace_YUV:
            return MHW_CSpace_YUV;

        case CSpace_Gray:
            return MHW_CSpace_Gray;

        case CSpace_Any:
            return MHW_CSpace_Any;

        case CSpace_sRGB:
            return MHW_CSpace_sRGB;

        case CSpace_stRGB:
            return MHW_CSpace_stRGB;

        case CSpace_BT601:
            return MHW_CSpace_BT601;

        case CSpace_BT601_FullRange:
            return MHW_CSpace_BT601_FullRange;

        case CSpace_BT709:
            return MHW_CSpace_BT709;

        case CSpace_BT709_FullRange:
            return MHW_CSpace_BT709_FullRange;

        case CSpace_xvYCC601:
            return MHW_CSpace_xvYCC601;

        case CSpace_xvYCC709:
            return MHW_CSpace_xvYCC709;

        case CSpace_BT601Gray:
            return MHW_CSpace_BT601Gray;

        case CSpace_BT601Gray_FullRange:
            return MHW_CSpace_BT601Gray_FullRange;

        case CSpace_BT2020:
            return MHW_CSpace_BT2020;

        case CSpace_BT2020_RGB:
            return MHW_CSpace_BT2020_RGB;

        case CSpace_BT2020_FullRange:
            return MHW_CSpace_BT2020_FullRange;

        case CSpace_BT2020_stRGB:
            return MHW_CSpace_BT2020_stRGB;

        case CSpace_None:
        default:
            return MHW_CSpace_None;
    }
}

MOS_STATUS VpHal_InitVeboxSurfaceParams(
    PVPHAL_SURFACE                 pVpHalVeboxSurface,
    PMHW_VEBOX_SURFACE_PARAMS      pMhwVeboxSurface)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL(pVpHalVeboxSurface);
    VPHAL_RENDER_CHK_NULL(pMhwVeboxSurface);

    MOS_ZeroMemory(pMhwVeboxSurface, sizeof(*pMhwVeboxSurface));
    pMhwVeboxSurface->bActive                = true;
    pMhwVeboxSurface->Format                 = pVpHalVeboxSurface->Format;
    pMhwVeboxSurface->dwWidth                = pVpHalVeboxSurface->dwWidth;
    pMhwVeboxSurface->dwHeight               = pVpHalVeboxSurface->dwHeight;
    pMhwVeboxSurface->dwPitch                = pVpHalVeboxSurface->dwPitch;
    pMhwVeboxSurface->dwBitDepth             = pVpHalVeboxSurface->dwDepth;
    pMhwVeboxSurface->TileType               = pVpHalVeboxSurface->TileType;
    pMhwVeboxSurface->TileModeGMM            = pVpHalVeboxSurface->TileModeGMM;
    pMhwVeboxSurface->bGMMTileEnabled        = pVpHalVeboxSurface->bGMMTileEnabled;
    pMhwVeboxSurface->rcMaxSrc               = pVpHalVeboxSurface->rcMaxSrc;
    pMhwVeboxSurface->pOsResource            = &pVpHalVeboxSurface->OsResource;
    pMhwVeboxSurface->bIsCompressed          = pVpHalVeboxSurface->bIsCompressed;
    pMhwVeboxSurface->dwCompressionFormat    = pVpHalVeboxSurface->CompressionFormat;

    if (pVpHalVeboxSurface->dwPitch > 0)
    {
        pMhwVeboxSurface->dwUYoffset = ((pVpHalVeboxSurface->UPlaneOffset.iSurfaceOffset - pVpHalVeboxSurface->YPlaneOffset.iSurfaceOffset) / pVpHalVeboxSurface->dwPitch) + pVpHalVeboxSurface->UPlaneOffset.iYOffset;
    }

finish:
    return eStatus;
}

MOS_STATUS VpHal_InitVeboxSurfaceStateCmdParams(
    PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    pVpHalVeboxSurfaceStateCmdParams,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS      pMhwVeboxSurfaceStateCmdParams)
{
    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL(pVpHalVeboxSurfaceStateCmdParams);
    VPHAL_RENDER_CHK_NULL(pMhwVeboxSurfaceStateCmdParams);

    MOS_ZeroMemory(pMhwVeboxSurfaceStateCmdParams, sizeof(*pMhwVeboxSurfaceStateCmdParams));

    pMhwVeboxSurfaceStateCmdParams->bDIEnable       = pVpHalVeboxSurfaceStateCmdParams->bDIEnable;

    if (pVpHalVeboxSurfaceStateCmdParams->pSurfInput)
    {
        VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(
                                pVpHalVeboxSurfaceStateCmdParams->pSurfInput,
                                &pMhwVeboxSurfaceStateCmdParams->SurfInput));
        pMhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfInput->YPlaneOffset.iYOffset;
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfOutput)
    {
        pMhwVeboxSurfaceStateCmdParams->bOutputValid = true;
        VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(
                                pVpHalVeboxSurfaceStateCmdParams->pSurfOutput,
                                &pMhwVeboxSurfaceStateCmdParams->SurfOutput));
        pMhwVeboxSurfaceStateCmdParams->SurfOutput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->YPlaneOffset.iYOffset;
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM)
    {
        VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(
                                pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM,
                                &pMhwVeboxSurfaceStateCmdParams->SurfSTMM));
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput)
    {
        VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(
                                pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput,
                                &pMhwVeboxSurfaceStateCmdParams->SurfDNOutput));
    }

    if (pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput)
    {
        VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(
                                pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput,
                                &pMhwVeboxSurfaceStateCmdParams->SurfSkinScoreOutput));
    }

finish:
    return eStatus;
}
