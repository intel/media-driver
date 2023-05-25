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
//! \file     vp_feature_manager_xe_xpm_base.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "vp_feature_manager_xe_xpm_base.h"
#include "mhw_vebox_xe_xpm.h"
#include "vp_user_feature_control.h"
using namespace vp;

/****************************************************************************************************/
/*                                    VPFeatureManagerXe_Xpm_Base                                      */
/****************************************************************************************************/

VPFeatureManagerXe_Xpm_Base::VPFeatureManagerXe_Xpm_Base(
    PVP_MHWINTERFACE  hwInterface) :
    VPFeatureManager(hwInterface)
{

}

MOS_STATUS VPFeatureManagerXe_Xpm_Base::IsScalabilityNeeded(bool &isScalabilityNeeded, uint32_t srcWidth, uint32_t srcHeight)
{
    VP_FUNC_CALL();

    PMOS_INTERFACE          pOsInterface    = nullptr;
    PMHW_VEBOX_INTERFACE    pVeboxState     = nullptr;

    isScalabilityNeeded = false;

    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_osInterface);

    pOsInterface    = m_hwInterface->m_osInterface;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pOsInterface->bEnableDbgOvrdInVE)
    {
        if (pOsInterface->bVeboxScalabilityMode)
        {
            isScalabilityNeeded = true;
        }
        return MOS_STATUS_SUCCESS;
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VPFeatureManagerXe_Xpm_Base::CheckFeatures(void * params, bool &bApgFuncSupported)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(params);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_osInterface);

    auto userFeatureControl = m_hwInterface->m_userFeatureControl;
    bool disableVeboxOutput = userFeatureControl->IsVeboxOutputDisabled();
    bool disableSfc = userFeatureControl->IsSfcDisabled();
    PVP_PIPELINE_PARAMS pvpParams = (PVP_PIPELINE_PARAMS)params;
    bApgFuncSupported = false;

    if (!m_hwInterface->m_osInterface->apoMosEnabled)
    {
        VP_PUBLIC_NORMALMESSAGE("Fallback to legacy since APO mos not enabled.");
        return MOS_STATUS_SUCCESS;
    }

    // If bForceToRender is set as true, then output pipe is decided by whether HDR is enabled.
    // If HDR is enabled, use APO path. Otherwise legacy path is selected.
    if (disableVeboxOutput && disableSfc && !pvpParams->bForceToRender)
    {
        VP_PUBLIC_NORMALMESSAGE("Fallback to legacy since 1. both vebox output and sfc being disabled. 2. Render is not forced to");
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

    // WA: Force NV12 16K to render
    if (pvpParams->pTarget[0]->Format == Format_NV12 && pvpParams->pTarget[0]->dwHeight > VPHAL_RNDR_16K_HEIGHT_LIMIT)
    {
        VP_RENDER_NORMALMESSAGE("Disable VEBOX/SFC for NV12 16k resolution");
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

    if (pvpParams->pSrc[0]->pBlendingParams                 ||
        pvpParams->pSrc[0]->pLumaKeyParams                  ||
        pvpParams->pConstriction)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (pvpParams->pSrc[0]->bInterlacedScaling && !IsSfcInterlacedScalingSupported())
    {
        return MOS_STATUS_SUCCESS;
    }

    if (nullptr == pvpParams->pSrc[0]->pHDRParams && Is2PassesCSCNeeded(pvpParams->pSrc[0], pvpParams->pTarget[0]))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Temp removed RGB input with DN/DI/IECP case
    if ((IS_RGB_FORMAT(pvpParams->pSrc[0]->Format)) &&
        (pvpParams->pSrc[0]->pColorPipeParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (!IsVeboxOutFeasible(pvpParams) &&
        !IsSfcOutputFeasible(pvpParams))
    {
        bool isHdrNeeded = IsHdrNeeded(pvpParams->pSrc[0], pvpParams->pTarget[0]);

        if (!isHdrNeeded)
        {
            return MOS_STATUS_SUCCESS;
        }
        VP_PUBLIC_NORMALMESSAGE("Enable APG for HDR.");
    }

    bool bVeboxNeeded = IsVeboxSupported(pvpParams);
    // If ScalingPreference == VPHAL_SCALING_PREFER_SFC_FOR_VEBOX, use SFC only when VEBOX is required
    // For GEN12+, IEF has been removed from AVS sampler. Do not change the path if IEF is enabled.
    if ((pvpParams->pSrc[0]->ScalingPreference == VPHAL_SCALING_PREFER_SFC_FOR_VEBOX) &&
        (pvpParams->pSrc[0]->pIEFParams == nullptr || (pvpParams->pSrc[0]->pIEFParams && pvpParams->pSrc[0]->pIEFParams->bEnabled == false)) &&
        (bVeboxNeeded == false))
    {
        VP_PUBLIC_NORMALMESSAGE("DDI choose to use SFC only for VEBOX, and since VEBOX is not required, change to Composition.");
        return MOS_STATUS_SUCCESS;
    }

    if (pvpParams->pSrc[0]->ScalingPreference == VPHAL_SCALING_PREFER_COMP)
    {
        if (pvpParams->pSrc[0]->pDenoiseParams &&
            pvpParams->pSrc[0]->pDenoiseParams->bEnableHVSDenoise)
        {
            VP_PUBLIC_NORMALMESSAGE("If HVS case, still go to APG path, not need change to Composition");
        }
        else if (IsHdrNeeded(pvpParams->pSrc[0], pvpParams->pTarget[0]))
        {
            VP_PUBLIC_NORMALMESSAGE("If HDR case, still go to APG path, not need change to Composition");
        }
        else if (IS_COLOR_SPACE_BT2020_YUV(pvpParams->pSrc[0]->ColorSpace) && IS_COLOR_SPACE_BT2020_YUV(pvpParams->pTarget[0]->ColorSpace))
        {
            VP_PUBLIC_NORMALMESSAGE("If input color space is BT2020 and output color space is BT2020_FullRange, go to APG path.");
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("DDI choose to use Composition, change to Composition.");
            return MOS_STATUS_SUCCESS;
        }
    }

    bApgFuncSupported = true;

    return MOS_STATUS_SUCCESS;
}

bool VPFeatureManagerXe_Xpm_Base::IsRGBOutputFormatSupported(
    PVPHAL_SURFACE outSurface)
{
    VP_FUNC_CALL();

    if (VPFeatureManager::IsRGBOutputFormatSupported(outSurface) ||
        IS_RGB64_FORMAT(outSurface->Format))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VPFeatureManagerXe_Xpm_Base::IsOutputFormatSupported(
    PVPHAL_SURFACE              outSurface)
{
    VP_FUNC_CALL();

    bool ret = true;

    if (IsRGBOutputFormatSupported(outSurface) ||
        outSurface->Format == Format_NV12      ||
        outSurface->Format == Format_YUY2      ||
        outSurface->Format == Format_UYVY      ||
        outSurface->Format == Format_AYUV      ||
        outSurface->Format == Format_Y210      ||
        outSurface->Format == Format_Y410      ||
        outSurface->Format == Format_Y216      ||
        outSurface->Format == Format_Y416      ||
        outSurface->Format == Format_VYUY      ||
        outSurface->Format == Format_YVYU      ||
        outSurface->Format == Format_Y8        ||
        outSurface->Format == Format_Y16S      ||
        outSurface->Format == Format_Y16U)
    {
        ret = true;
    }
    else if (VPFeatureManager::IsNV12P010OutputFormatSupported(outSurface))
    {
        ret = true;
    }
    else
    {
        VP_RENDER_NORMALMESSAGE("Unsupported Render Target Format '0x%08x' for SFC Pipe.", outSurface->Format);
        ret = false;
    }

    return ret;
}
