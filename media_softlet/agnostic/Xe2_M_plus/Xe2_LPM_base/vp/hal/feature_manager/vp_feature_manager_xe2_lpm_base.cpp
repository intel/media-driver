/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_feature_manager_xe2_lpm_base.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "vp_feature_manager_xe2_lpm_base.h"
#include "mhw_vebox_xe2_lpm_base_next_impl.h"
using namespace vp;

/****************************************************************************************************/
/*                                    VPFeatureManagerXe2_Lpm_Base                                      */
/****************************************************************************************************/

VPFeatureManagerXe2_Lpm_Base::VPFeatureManagerXe2_Lpm_Base(
    PVP_MHWINTERFACE  hwInterface) :
    VPFeatureManager(hwInterface)
{

}

MOS_STATUS VPFeatureManagerXe2_Lpm_Base::IsScalabilityNeeded(bool &isScalabilityNeeded, uint32_t srcWidth, uint32_t srcHeight)
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

    if (srcWidth > MHW_VEBOX_4K_PIC_WIDTH_XE2_LPM_BASE &&
        srcHeight > MHW_VEBOX_4K_PIC_HEIGHT_XE2_LPM_BASE)
    {
        isScalabilityNeeded = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VPFeatureManagerXe2_Lpm_Base::CheckFeatures(void *params, bool &bApgFuncSupported)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(params);

    PVP_PIPELINE_PARAMS pvpParams           = (PVP_PIPELINE_PARAMS)params;

    MOS_STATUS status = VPFeatureManager::CheckFeatures(params, bApgFuncSupported);

    if (MOS_FAILED(status) || !bApgFuncSupported)
    {
        return status;
    }

    return MOS_STATUS_SUCCESS;
}

bool VPFeatureManagerXe2_Lpm_Base::IsNV12P010OutputFormatSupported(
    PVPHAL_SURFACE outSurface)
{
    VP_FUNC_CALL();

    if ((outSurface->TileType == MOS_TILE_Y ||
         (MEDIA_IS_SKU(m_hwInterface->m_skuTable, FtrSFC420LinearOutputSupport) &&
         outSurface->TileType == MOS_TILE_LINEAR)) &&
        (outSurface->Format == Format_P010 ||
         outSurface->Format == Format_P016 ||
         outSurface->Format == Format_NV12))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VPFeatureManagerXe2_Lpm_Base::IsRGBOutputFormatSupported(
    PVPHAL_SURFACE outSurface)
{
    VP_FUNC_CALL();

    if (VPFeatureManager::IsRGBOutputFormatSupported(outSurface) ||
        IS_RGB64_FORMAT(outSurface->Format) ||
        IS_PL3_RGB_FORMAT(outSurface->Format) && MEDIA_IS_SKU(m_hwInterface->m_skuTable, FtrSFCRGBPRGB24OutputSupport) ||
        IS_RGB24_FORMAT(outSurface->Format) && MEDIA_IS_SKU(m_hwInterface->m_skuTable, FtrSFCRGBPRGB24OutputSupport))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VPFeatureManagerXe2_Lpm_Base::IsOutputFormatSupported(
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
    else if (IsNV12P010OutputFormatSupported(outSurface))
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
