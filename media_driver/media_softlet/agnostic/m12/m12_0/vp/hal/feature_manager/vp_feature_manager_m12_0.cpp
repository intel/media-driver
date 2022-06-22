/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     vp_feature_manager_m12_0.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "vp_feature_manager_m12_0.h"
using namespace vp;

/****************************************************************************************************/
/*                                      VPFeatureManagerM12_0                                       */
/****************************************************************************************************/

VPFeatureManagerM12_0::VPFeatureManagerM12_0(
    PVP_MHWINTERFACE  hwInterface) :
    VPFeatureManager(hwInterface)
{
}

bool VPFeatureManagerM12_0::IsVeboxInputFormatSupport(PVPHAL_SURFACE pSrcSurface)
{
    VP_FUNC_CALL();

    bool    bRet = false;
    VP_RENDER_CHK_NULL_NO_STATUS(pSrcSurface);

    // Check if Sample Format is supported
    // Vebox only support P016 format, P010 format can be supported by faking it as P016
    if (pSrcSurface->Format != Format_NV12 &&
        pSrcSurface->Format != Format_AYUV &&
        pSrcSurface->Format != Format_P010 &&
        pSrcSurface->Format != Format_P016 &&
        pSrcSurface->Format != Format_P210 &&
        pSrcSurface->Format != Format_P216 &&
        pSrcSurface->Format != Format_Y8 &&
        pSrcSurface->Format != Format_Y16U &&
        pSrcSurface->Format != Format_Y16S &&
        !IS_PA_FORMAT(pSrcSurface->Format)/* &&
        !IS_RGB64_FLOAT_FORMAT(pSrcSurface->Format)*/)
    {
        VP_RENDER_NORMALMESSAGE("Unsupported Source Format '0x%08x' for VEBOX.", pSrcSurface->Format);
        goto finish;
    }

    bRet = true;

finish:
    return bRet;
}

MOS_STATUS VPFeatureManagerM12_0::CheckFeatures(void * params, bool &bApgFuncSupported)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(params);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_osInterface);

    PVP_PIPELINE_PARAMS pvpParams = (PVP_PIPELINE_PARAMS)params;
    bApgFuncSupported = false;

    if (!m_hwInterface->m_osInterface->apoMosEnabled)
    {
        VP_PUBLIC_NORMALMESSAGE("Fallback to legacy since APO mos not enabled.");
        return MOS_STATUS_SUCCESS;
    }

    // APG only support single frame input/output
    if (pvpParams->uSrcCount != 1 ||
        pvpParams->uDstCount != 1)
    {
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_CHK_NULL_RETURN(pvpParams->pSrc[0]);
    VP_PUBLIC_CHK_NULL_RETURN(pvpParams->pTarget[0]);

    if (pvpParams->pSrc[0]->SurfType != SURF_IN_PRIMARY)
    {
        return MOS_STATUS_SUCCESS;
    }

    // align rectangle of surface
    VP_PUBLIC_CHK_STATUS_RETURN(RectSurfaceAlignment(pvpParams->pSrc[0], pvpParams->pTarget[0]->Format));
    VP_PUBLIC_CHK_STATUS_RETURN(RectSurfaceAlignment(pvpParams->pTarget[0], pvpParams->pTarget[0]->Format));

    //Force 8K to render. Handle this case in APG path after render path being enabled.
    if (pvpParams->bDisableVeboxFor8K &&
        ((pvpParams->pSrc[0]->dwWidth >= VPHAL_RNDR_8K_WIDTH || pvpParams->pSrc[0]->dwHeight >= VPHAL_RNDR_8K_HEIGHT) ||
         (pvpParams->pTarget[0]->dwWidth >= VPHAL_RNDR_8K_WIDTH || pvpParams->pTarget[0]->dwHeight >= VPHAL_RNDR_8K_HEIGHT)))
    {
        VP_PUBLIC_NORMALMESSAGE("Disable VEBOX/SFC for 8k resolution");
        return MOS_STATUS_SUCCESS;
    }

    if (IsHdrNeeded(pvpParams->pSrc[0], pvpParams->pTarget[0]))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Check whether VEBOX is available
    // VTd doesn't support VEBOX
    if (!MEDIA_IS_SKU(m_hwInterface->m_skuTable, FtrVERing))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Check if the Surface size is greater than 64x16 which is the minimum Width and Height VEBOX can handle
    if (pvpParams->pSrc[0]->dwWidth < MHW_VEBOX_MIN_WIDTH || pvpParams->pSrc[0]->dwHeight < MHW_VEBOX_MIN_HEIGHT)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (pvpParams->pSrc[0]->pDeinterlaceParams              ||
        pvpParams->pSrc[0]->pBlendingParams                 ||
        pvpParams->pSrc[0]->pColorPipeParams                ||
        pvpParams->pSrc[0]->pHDRParams                      ||
        pvpParams->pSrc[0]->pLumaKeyParams                  ||
        pvpParams->pSrc[0]->pProcampParams                  ||
        pvpParams->pSrc[0]->bInterlacedScaling              ||
        pvpParams->pConstriction)
    {
        return MOS_STATUS_SUCCESS;
    }

    // Disable chroma DN in APO path.
    // Disable HVS Denoise in APO path.
    if (pvpParams->pSrc[0]->pDenoiseParams                       &&
       (pvpParams->pSrc[0]->pDenoiseParams->bEnableChroma        ||
        pvpParams->pSrc[0]->pDenoiseParams->bEnableHVSDenoise))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (pvpParams->pSrc[0]->p3DLutParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (Is2PassesCSCNeeded(pvpParams->pSrc[0], pvpParams->pTarget[0]))
    {
        return MOS_STATUS_SUCCESS;
    }

    // for now, Temp removed ARGB input for APG
    if (pvpParams->pSrc[0]->Format == Format_A8R8G8B8 ||
        pvpParams->pSrc[0]->Format == Format_X8R8G8B8)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool bVeboxNeeded = false;
    bool bSFCNeeded   = IsSfcOutputFeasible(pvpParams);

    if (IsVeboxOutFeasible(pvpParams) ||
        !bSFCNeeded)
    {
        return MOS_STATUS_SUCCESS;
    }

    if ((!bVeboxNeeded &&
        pvpParams->pSrc[0]->ScalingPreference == VPHAL_SCALING_PREFER_SFC_FOR_VEBOX) ||
        pvpParams->pSrc[0]->ScalingPreference == VPHAL_SCALING_PREFER_COMP)
    {
        VP_PUBLIC_NORMALMESSAGE("DDI choose to use SFC only for VEBOX, and since VEBOX is not required, change to Composition.");
        return MOS_STATUS_SUCCESS;
    }

    bApgFuncSupported = true;

    return MOS_STATUS_SUCCESS;
}