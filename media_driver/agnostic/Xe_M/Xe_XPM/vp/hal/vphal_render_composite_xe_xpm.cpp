/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     vphal_render_composite_xe_xpm.cpp
//! \brief    Composite related VPHAL functions
//! \details  Unified VP HAL Composite module including render initialization,
//!           resource allocation/free and rendering
//!
#include "vphal_render_composite_xe_xpm.h"
#include "vp_hal_ddi_utils.h"

#define VPHAL_SAMPLER_Y1                 4
#define VPHAL_SAMPLER_U1                 5
#define VPHAL_SAMPLER_V1                 6
#define VPHAL_COMP_SOURCE_DEPTH          16
#define VPHAL_COMP_P010_DEPTH            0

const int32_t Samplerindex[2][3] = { {VPHAL_SAMPLER_Y, VPHAL_SAMPLER_U, VPHAL_SAMPLER_V },
                                     {VPHAL_SAMPLER_Y1, VPHAL_SAMPLER_U1, VPHAL_SAMPLER_V1}};
const uint8_t SamplerSelectionArray[2][8] = { {0, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 1, 1, 1, 1, 1, 1, 1}};

MOS_STATUS CompositeStateXe_Xpm ::GetSamplerIndex(
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE_STATE_ENTRY      pEntry,
    int32_t*                            pSamplerIndex,
    PMHW_SAMPLER_TYPE                   pSamplerType)
{
    if (pSurface == nullptr || pSamplerIndex == nullptr || pSamplerType == nullptr || pEntry == nullptr)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid parameter.");
        return MOS_STATUS_NULL_POINTER;
    }

    // reset Scaling mode to NEAREST due to XeHP don't have AVS
    if (pSurface->ScalingMode == VPHAL_SCALING_AVS)
    {
        pSurface->ScalingMode = VPHAL_SCALING_BILINEAR;
    }

    // if Scalingmode is BILINEAR, use the 4,5,6. if NEAREST, use 1,2,3
    pEntry->bAVS = false;
    *pSamplerType = MHW_SAMPLER_TYPE_3D;

    *pSamplerIndex = Samplerindex[pSurface->ScalingMode][pEntry->YUVPlane];

    return MOS_STATUS_SUCCESS;
}

bool CompositeStateXe_Xpm ::IsSamplerIDForY(int32_t    SamplerID)
{
    return ((SamplerID == VPHAL_SAMPLER_Y) || (SamplerID == VPHAL_SAMPLER_Y1)) ? true : false;
}

MOS_STATUS CompositeStateXe_Xpm ::Set3DSamplerStatus(
    PVPHAL_SURFACE                 pSurface,
    uint8_t                        Layer,
    MEDIA_OBJECT_KA2_STATIC_DATA   *pStatic)
{
    if (pStatic == nullptr || pSurface  == nullptr || Layer > 7)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid parameter.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    // reset Scaling mode to NEAREST due to XeHP don't have AVS
    if (pSurface->ScalingMode == VPHAL_SCALING_AVS)
    {
        pSurface->ScalingMode = VPHAL_SCALING_BILINEAR;
    }

    // Set which layer's Sampler status' selection: MEDIA_OBJECT_KA2_STATIC_DATA's DW14's bit 24 to 31.
    pStatic->DW14.Value |= (SamplerSelectionArray[pSurface->ScalingMode][Layer] << (Layer+24));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CompositeStateXe_Xpm ::UpdateInlineDataStatus(
    PVPHAL_SURFACE                 pSurface,
    MEDIA_OBJECT_KA2_STATIC_DATA   *pStatic)
{
    uint32_t                      uiBitDepth = 0;

    if (pStatic == nullptr || pSurface  == nullptr)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid parameter.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uiBitDepth                = VpHalDDIUtils::GetSurfaceBitDepth(pSurface->Format);
    pStatic->DW07.OutputDepth = VPHAL_COMP_P010_DEPTH;
    if (uiBitDepth && !(pSurface->Format == Format_P010 || pSurface->Format == Format_Y210))
    {
        pStatic->DW07.OutputDepth = VPHAL_COMP_SOURCE_DEPTH - uiBitDepth;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Get intermediate surface output
//! \param    pOutput
//!           [in] Pointer to Intermediate Output Surface
//! \return   PVPHAL_SURFACE
//!           Return the chose output
//!
MOS_STATUS CompositeStateXe_Xpm::GetIntermediateOutput(PVPHAL_SURFACE &output)
{
    if (output == m_Intermediate)
    {
        output = m_Intermediate1;
    }
    else
    {
        output = m_Intermediate;
    }
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Decompress the Surface
//! \details  Decompress the interlaced Surface which is in the RC compression mode
//! \param    [in,out] pSource
//!           Pointer to Source Surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS CompositeStateXe_Xpm::DecompressInterlacedSurf(PVPHAL_SURFACE pSource)
{
    VPHAL_RENDER_CHK_NULL_RETURN(pSource);

    // Interlaced surface in the compression mode needs to decompress
    if ((pSource->CompressionMode == MOS_MMC_MC                            ||
         pSource->CompressionMode == MOS_MMC_RC)                           &&
        (pSource->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD    ||
         pSource->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD ||
         pSource->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD     ||
         pSource->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD))
    {
        VPHAL_RENDER_CHK_NULL_RETURN(m_pOsInterface);
        bool bAllocated = false;

        //Use auxiliary surface to sync with decompression
        VPHAL_RENDER_CHK_STATUS_RETURN(VpHal_ReAllocateSurface(
            m_pOsInterface,
            &m_AuxiliarySyncSurface,
            "AuxiliarySyncSurface",
            Format_Buffer,
            MOS_GFXRES_BUFFER,
            MOS_TILE_LINEAR,
            32,
            1,
            false,
            MOS_MMC_DISABLED,
            &bAllocated));
      
        VPHAL_RENDER_CHK_STATUS_RETURN(m_pOsInterface->pfnSetDecompSyncRes(m_pOsInterface, &m_AuxiliarySyncSurface.OsResource));
        VPHAL_RENDER_CHK_STATUS_RETURN(m_pOsInterface->pfnDecompResource(m_pOsInterface, &pSource->OsResource));
        VPHAL_RENDER_CHK_STATUS_RETURN(m_pOsInterface->pfnSetDecompSyncRes(m_pOsInterface, nullptr));
        VPHAL_RENDER_CHK_STATUS_RETURN(m_pOsInterface->pfnRegisterResource(m_pOsInterface, &m_AuxiliarySyncSurface.OsResource, true, true));

        MOS_SURFACE osSurface = {};
        VPHAL_RENDER_CHK_STATUS_RETURN(m_pOsInterface->pfnGetResourceInfo(m_pOsInterface, &pSource->OsResource, &osSurface));
        // vphal_surface do not have m_mmcstate, so no need to update m_mmcstate
        pSource->bIsCompressed     = osSurface.bIsCompressed;
        pSource->CompressionMode   = osSurface.CompressionMode;
        pSource->CompressionFormat = osSurface.CompressionFormat;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CompositeStateXe_Xpm ::SetSamplerFilterMode(
    PMHW_SAMPLER_STATE_PARAM&       pSamplerStateParams,
    PRENDERHAL_SURFACE_STATE_ENTRY  pEntry,
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderData,
    uint32_t                        uLayerNum,
    MHW_SAMPLER_FILTER_MODE         SamplerFilterMode,
    int32_t*                        pSamplerIndex,
    PVPHAL_SURFACE                  pSource)
{
    VPHAL_RENDER_CHK_NULL_RETURN(pEntry);
    VPHAL_RENDER_CHK_NULL_RETURN(pSamplerIndex);
    VPHAL_RENDER_CHK_NULL_RETURN(pSamplerStateParams);
    VPHAL_RENDER_CHK_NULL_RETURN(pRenderData);

    if (pSource == nullptr || uLayerNum < 2)
    {
        pSamplerStateParams->Unorm.SamplerFilterMode = SamplerFilterMode;
        return MOS_STATUS_SUCCESS;
    }

    if (MHW_SAMPLER_FILTER_BILINEAR == SamplerFilterMode &&
            VPHAL_SCALING_BILINEAR != pSource->ScalingMode ||
        MHW_SAMPLER_FILTER_NEAREST == SamplerFilterMode &&
            VPHAL_SCALING_NEAREST != pSource->ScalingMode)
    {
        pSource->ScalingMode                         = (MHW_SAMPLER_FILTER_BILINEAR == SamplerFilterMode) ? VPHAL_SCALING_BILINEAR : VPHAL_SCALING_NEAREST;
        VPHAL_RENDER_CHK_STATUS_RETURN(GetSamplerIndex(pSource, pEntry, pSamplerIndex, &pSamplerStateParams->SamplerType));
        pSamplerStateParams                          = &pRenderData->SamplerStateParams[*pSamplerIndex];
        pSamplerStateParams->SamplerType             = MHW_SAMPLER_TYPE_3D;

        VPHAL_RENDER_NORMALMESSAGE("Updating scaling mode %d and SamplerIndex %d", pSource->ScalingMode, *pSamplerIndex);
    }
    pSamplerStateParams->Unorm.SamplerFilterMode = SamplerFilterMode;

    return MOS_STATUS_SUCCESS;
}
