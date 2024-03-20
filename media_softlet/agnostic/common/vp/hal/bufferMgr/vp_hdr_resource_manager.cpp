/* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_hdr_resource_manager.cpp
//! \brief    The source file of the extention class of vp hdr resource manager
//! \details  all the vp hdr resources will be traced here for usages using intermeida
//!           surfaces.
//!

#include "vp_hdr_resource_manager.h"
#include "vp_allocator.h"
#include "vp_hdr_filter.h"

using namespace vp;

VphdrResourceManager::VphdrResourceManager(VpAllocator &allocator) : m_allocator(allocator)
{
    VP_FUNC_CALL();
}

VphdrResourceManager::~VphdrResourceManager()
{
    VP_FUNC_CALL();
    FreeHdrRenderResource();
}

MOS_STATUS VphdrResourceManager::AssignRenderResource(VP_EXECUTE_CAPS &caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface, RESOURCE_ASSIGNMENT_HINT resHint,
    VP_SURFACE_SETTING &surfSetting, SwFilterPipe &executedFilters, MOS_INTERFACE &osInterface, VphalFeatureReport &reporting, bool deferredDestroyed)
{
    VP_FUNC_CALL();

    bool allocated = false;
    auto        *skuTable            = osInterface.pfnGetSkuTable(&osInterface);
    Mos_MemPool memTypeSurfVideoMem = MOS_MEMPOOL_VIDEOMEMORY;
    uint32_t    dwWidth             = 0;
    uint32_t    dwHeight            = 0;
    uint32_t    dwDepth             = 0;
    bool        bAllocated          = false;
    SwFilterHdr *hdr      = dynamic_cast<SwFilterHdr *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeHdrOnRender));
    VP_PUBLIC_CHK_NULL_RETURN(hdr);
    auto         params     = hdr->GetSwFilterParams();
    MOS_ALLOC_GFXRES_PARAMS AllocParams  = {};
    VPHAL_GET_SURFACE_INFO  Info         = {};
    MOS_STATUS   eStatus;

    if (skuTable && MEDIA_IS_SKU(skuTable, FtrLimitedLMemBar))
    {
        memTypeSurfVideoMem = MOS_MEMPOOL_DEVICEMEMORY;
    }

    // Allocate CSC CCM Coeff Surface
    dwWidth  = VPHAL_HDR_COEF_SURFACE_WIDTH;
    dwHeight = VPHAL_HDR_COEF_SURFACE_HEIGHT;

    surfSetting.pHDRStageConfigTable = HDRStageConfigTable;

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_hdrCoeff,
        "HdrCoeffSurface",
        Format_A8R8G8B8,
        MOS_GFXRES_2D,
        MOS_TILE_LINEAR,
        dwWidth,
        dwHeight,
        false,
        MOS_MMC_DISABLED,
        allocated,
        false,
        deferredDestroyed,
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER));

    surfSetting.coeffAllocated = allocated;
    surfSetting.surfGroup.insert(std::make_pair(SurfaceTypeHdrCoeff, m_hdrCoeff));

    // Allocate auto mode CSC CCM Coeff Surface
    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_hdrAutoModeCoeffSurface,
        "AutoModeCoeffSurface",
        Format_A8R8G8B8,
        MOS_GFXRES_2D,
        MOS_TILE_LINEAR,
        dwWidth,
        dwHeight,
        false,
        MOS_MMC_DISABLED,
        allocated,
        false,
        deferredDestroyed,
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER));

    surfSetting.surfGroup.insert(std::make_pair(SurfaceTypeHdrAutoModeCoeff, m_hdrAutoModeCoeffSurface));

    // Allocate auto mode iir temp Surface
    dwWidth  = VPHAL_HDR_AUTO_MODE_IIR_TEMP_SIZE;
    dwHeight = 1;

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_hdrAutoModeIirTempSurface,
        "AutoModeIirTempSurface",
        Format_L8,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        dwWidth,
        dwHeight,
        false,
        MOS_MMC_DISABLED,
        allocated,
        false,
        deferredDestroyed,
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER));

    surfSetting.surfGroup.insert(std::make_pair(SurfaceTypeHdrAutoModeIirTempSurface, m_hdrAutoModeIirTempSurface));

    // Allocate OETF 1D LUT Surface
    dwWidth  = VPHAL_HDR_OETF_1DLUT_WIDTH;
    dwHeight = VPHAL_HDR_OETF_1DLUT_WIDTH;

    size_t cnt = MOS_MIN(inputSurfaces.size(), VPHAL_MAX_HDR_INPUT_LAYER);
    for (size_t i = 0; i < cnt; ++i)
    {
        surfSetting.surfGroup.insert(std::make_pair((SurfaceType)(SurfaceTypeHdrInputLayer0 + i), inputSurfaces[i]));

        SwFilterHdr    *hdr    = dynamic_cast<SwFilterHdr *>(executedFilters.GetSwFilter(true, i, FeatureType::FeatureTypeHdrOnRender));
        FeatureParamHdr params = {};
        if (hdr)
        {
            params = hdr->GetSwFilterParams();
        }
        
        // If LUTMode is not 2D, then there is no need to allocate OETF LUT surface.
        // One exception is that, LUTMode is 3D, but bGpuGenerate3DLUT user feature key is set to force using GPU to generate 3D LUT.
        // In this case, it has to go through the full pipe first for 3D LUT generation.
        if ((params.lutMode != VPHAL_HDR_LUT_MODE_2D) &&
            !(params.lutMode == VPHAL_HDR_LUT_MODE_3D && params.bGpuGenerate3DLUT))
        {
            continue;
        }

        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
                m_hdrOETF1DLUTSurface[i],
                "OETF1DLUTSurface",
                Format_R16F,
                MOS_GFXRES_2D,
                MOS_TILE_LINEAR,
                dwWidth,
                dwHeight,
                false,
                MOS_MMC_DISABLED,
                allocated,
                false,
                deferredDestroyed,
                MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER));

        surfSetting.OETF1DLUTAllocated = allocated;
        surfSetting.surfGroup.insert(std::make_pair((SurfaceType)(SurfaceTypeHdrOETF1DLUTSurface0 + i), m_hdrOETF1DLUTSurface[i]));
    }

    dwWidth = dwHeight = dwDepth = VPHAL_HDR_CRI_3DLUT_SIZE;
    for (size_t i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; ++i)
    {
        SwFilterHdr    *hdr    = dynamic_cast<SwFilterHdr *>(executedFilters.GetSwFilter(true, i, FeatureType::FeatureTypeHdrOnRender));
        FeatureParamHdr params = {};
        if (hdr)
        {
            params = hdr->GetSwFilterParams();
        }
        if (params.lutMode != VPHAL_HDR_LUT_MODE_3D)
        {
            continue;
        }

         VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
             m_hdrCri3DLUTSurface[i],
             "Cri3DLUTSurface",
             params.bGpuGenerate3DLUT ? Format_R10G10B10A2 : Format_A16B16G16R16,
             MOS_GFXRES_VOLUME,
             MOS_TILE_LINEAR,
             dwWidth,
             dwHeight,
             false,
             MOS_MMC_DISABLED,
             allocated,
             false,
             deferredDestroyed,
             MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER,
             MOS_TILE_UNSET_GMM,
             MOS_MEMPOOL_VIDEOMEMORY,
             false,
             nullptr,
             dwDepth));

         surfSetting.Cri3DLUTAllocated = allocated;
         surfSetting.surfGroup.insert(std::make_pair((SurfaceType)(SurfaceTypeHdrCRI3DLUTSurface0 + i), m_hdrCri3DLUTSurface[i]));
    }

    surfSetting.surfGroup.insert(std::make_pair(SurfaceTypeHdrTarget0, outputSurface));
    surfSetting.dumpPostSurface = false;
    reporting.GetFeatures().hdrMode = params.hdrMode;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VphdrResourceManager::FreeHdrRenderResource()
{
    VP_FUNC_CALL();

    if (m_hdrCoeff)
    {
        m_allocator.DestroyVpSurface(m_hdrCoeff);
    }

    if (m_hdrAutoModeCoeffSurface)
    {
        m_allocator.DestroyVpSurface(m_hdrAutoModeCoeffSurface);
    }

    if (m_hdrAutoModeIirTempSurface)
    {
        m_allocator.DestroyVpSurface(m_hdrAutoModeIirTempSurface);
    }

    for (uint32_t i = 0; i < VP_MAX_HDR_INPUT_LAYER; i++)
    {
        if (m_hdrOETF1DLUTSurface[i])
        {
            m_allocator.DestroyVpSurface(m_hdrOETF1DLUTSurface[i]);
        }

        if (m_hdrCri3DLUTSurface[i])
        {
            m_allocator.DestroyVpSurface(m_hdrCri3DLUTSurface[i]);
        }
    }

    return MOS_STATUS_SUCCESS;
}
