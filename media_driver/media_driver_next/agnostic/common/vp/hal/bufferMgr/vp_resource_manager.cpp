/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     vp_resource_manager.cpp
//! \brief    The source file of the base class of vp resource manager
//! \details  all the vp resources will be traced here for usages using intermeida 
//!           surfaces.
//!
#include "vp_resource_manager.h"
#include "vp_vebox_cmd_packet.h"
#include "sw_filter_pipe.h"

using namespace std;
namespace vp
{

#define VP_SAME_SAMPLE_THRESHOLD 0

extern const VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION g_cInit_VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATIONS =
{
    // DWORD 0
    {
        {NOISE_BLF_RANGE_THRESHOLD_S0_DEFAULT},           // RangeThrStart0
    },

    // DWORD 1
    {
        {NOISE_BLF_RANGE_THRESHOLD_S1_DEFAULT},           // RangeThrStart1
    },

    // DWORD 2
    {
        {NOISE_BLF_RANGE_THRESHOLD_S2_DEFAULT},           // RangeThrStart2
    },

    // DWORD 3
    {
        {NOISE_BLF_RANGE_THRESHOLD_S3_DEFAULT},           // RangeThrStart3
    },

    // DWORD 4
    {
        {NOISE_BLF_RANGE_THRESHOLD_S4_DEFAULT},           // RangeThrStart4
    },

    // DWORD 5
    {
        {NOISE_BLF_RANGE_THRESHOLD_S5_DEFAULT},           // RangeThrStart5
    },

    // DWORD 6
    {
        {0},                                              // Reserved
    },

    // DWORD 7
    {
        {0},                                              // Reserved
    },

    // DWORD 8
    {
        {NOISE_BLF_RANGE_WGTS0_DEFAULT},                  // RangeWgt0
    },

    // DWORD 9
    {
        {NOISE_BLF_RANGE_WGTS1_DEFAULT},                  // RangeWgt1
    },

    // DWORD 10
    {
        {NOISE_BLF_RANGE_WGTS2_DEFAULT},                  // RangeWgt2
    },

    // DWORD 11
    {
        {NOISE_BLF_RANGE_WGTS3_DEFAULT},                  // RangeWgt3
    },

    // DWORD 12
    {
        {NOISE_BLF_RANGE_WGTS4_DEFAULT},                  // RangeWgt4
    },

    // DWORD 13
    {
        {NOISE_BLF_RANGE_WGTS5_DEFAULT},                  // RangeWgt5
    },

    // DWORD 14
    {
        {0},                                              // Reserved
    },

    // DWORD 15
    {
        {0},                                              // Reserved
    },

    // DWORD 16 - 41: DistWgt[5][5]
    {
        {NOISE_BLF_DISTANCE_WGTS00_DEFAULT, NOISE_BLF_DISTANCE_WGTS01_DEFAULT, NOISE_BLF_DISTANCE_WGTS02_DEFAULT, NOISE_BLF_DISTANCE_WGTS01_DEFAULT, NOISE_BLF_DISTANCE_WGTS00_DEFAULT},
        {NOISE_BLF_DISTANCE_WGTS10_DEFAULT, NOISE_BLF_DISTANCE_WGTS11_DEFAULT, NOISE_BLF_DISTANCE_WGTS12_DEFAULT, NOISE_BLF_DISTANCE_WGTS11_DEFAULT, NOISE_BLF_DISTANCE_WGTS10_DEFAULT},
        {NOISE_BLF_DISTANCE_WGTS20_DEFAULT, NOISE_BLF_DISTANCE_WGTS21_DEFAULT, NOISE_BLF_DISTANCE_WGTS22_DEFAULT, NOISE_BLF_DISTANCE_WGTS21_DEFAULT, NOISE_BLF_DISTANCE_WGTS20_DEFAULT},
        {NOISE_BLF_DISTANCE_WGTS10_DEFAULT, NOISE_BLF_DISTANCE_WGTS11_DEFAULT, NOISE_BLF_DISTANCE_WGTS12_DEFAULT, NOISE_BLF_DISTANCE_WGTS11_DEFAULT, NOISE_BLF_DISTANCE_WGTS10_DEFAULT},
        {NOISE_BLF_DISTANCE_WGTS00_DEFAULT, NOISE_BLF_DISTANCE_WGTS01_DEFAULT, NOISE_BLF_DISTANCE_WGTS02_DEFAULT, NOISE_BLF_DISTANCE_WGTS01_DEFAULT, NOISE_BLF_DISTANCE_WGTS00_DEFAULT},
    },

    // Padding
    {
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
    }
};

VpResourceManager::VpResourceManager(MOS_INTERFACE &osInterface, VpAllocator &allocator, VphalFeatureReport &reporting)
    : m_osInterface(osInterface), m_allocator(allocator), m_reporting(reporting)
{
}

VpResourceManager::~VpResourceManager()
{
    // Clean all intermedia Resource
    DestoryVeboxOutputSurface();
    DestoryVeboxDenoiseOutputSurface();

    for (uint32_t i = 0; i < VP_NUM_STMM_SURFACES; i++)
    {
        if (m_veboxSTMMSurface[i])
        {
            m_allocator.DestroyVpSurface(m_veboxSTMMSurface[i]);
        }
    }

    if (m_veboxStatisticsSurface)
    {
        m_allocator.DestroyVpSurface(m_veboxStatisticsSurface);
    }

    if (m_veboxRgbHistogram)
    {
        m_allocator.DestroyVpSurface(m_veboxRgbHistogram);
    }

    if (m_veboxDNTempSurface)
    {
        m_allocator.DestroyVpSurface(m_veboxDNTempSurface);
    }

    if (m_veboxDNSpatialConfigSurface)
    {
        m_allocator.DestroyVpSurface(m_veboxDNSpatialConfigSurface);
    }

    if (m_vebox3DLookUpTables)
    {
        m_allocator.DestroyVpSurface(m_vebox3DLookUpTables);
    }
}

MOS_STATUS VpResourceManager::StartProcessNewFrame(SwFilterPipe &pipe)
{
    VP_SURFACE *inputSurface    = pipe.GetSurface(true, 0);
    VP_SURFACE *outputSurface   = pipe.GetSurface(false, 0);
    SwFilter   *diFilter        = pipe.GetSwFilter(true, 0, FeatureTypeDi);

    if (nullptr == inputSurface && nullptr == outputSurface)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    VP_SURFACE *pastSurface = pipe.GetPastSurface(0);
    VP_SURFACE *futureSurface = pipe.GetFutureSurface(0);

    int32_t currentFrameId = inputSurface ? inputSurface->FrameID : (outputSurface ? outputSurface->FrameID : 0);
    int32_t pastFrameId = pastSurface ? pastSurface->FrameID : 0;
    int32_t futureFrameId = futureSurface ? futureSurface->FrameID : 0;

    m_pastFrameIds = m_currentFrameIds;

    m_currentFrameIds.valid                     = true;
    m_currentFrameIds.diEnabled                 = nullptr != diFilter;
    m_currentFrameIds.currentFrameId            = currentFrameId;
    m_currentFrameIds.pastFrameId               = pastFrameId;
    m_currentFrameIds.futureFrameId             = futureFrameId;
    m_currentFrameIds.pastFrameAvailable        = pastSurface ? true : false;
    m_currentFrameIds.futureFrameAvailable      = futureSurface ? true : false;


    // Only set sameSamples flag DI enabled frames.
    if (m_pastFrameIds.valid && m_currentFrameIds.pastFrameAvailable &&
        m_pastFrameIds.diEnabled && m_currentFrameIds.diEnabled)
    {
        m_sameSamples   =
               WITHIN_BOUNDS(
                      m_currentFrameIds.currentFrameId - m_pastFrameIds.currentFrameId,
                      -VP_SAME_SAMPLE_THRESHOLD,
                      VP_SAME_SAMPLE_THRESHOLD) &&
               WITHIN_BOUNDS(
                      m_currentFrameIds.pastFrameId - m_pastFrameIds.pastFrameId,
                      -VP_SAME_SAMPLE_THRESHOLD,
                      VP_SAME_SAMPLE_THRESHOLD);

        m_outOfBound    =
               OUT_OF_BOUNDS(
                      m_currentFrameIds.pastFrameId - m_pastFrameIds.currentFrameId,
                      -VP_SAME_SAMPLE_THRESHOLD,
                      VP_SAME_SAMPLE_THRESHOLD);
    }
    // bSameSamples flag also needs to be set for no reference case
    else if (m_pastFrameIds.valid && !m_currentFrameIds.pastFrameAvailable &&
        m_pastFrameIds.diEnabled && m_currentFrameIds.diEnabled)
    {
        m_sameSamples   =
               WITHIN_BOUNDS(
                      m_currentFrameIds.currentFrameId - m_pastFrameIds.currentFrameId,
                      -VP_SAME_SAMPLE_THRESHOLD,
                      VP_SAME_SAMPLE_THRESHOLD);
        m_outOfBound    = false;
    }
    else
    {
        m_sameSamples   = false;
        m_outOfBound    = false;
    }

    if (inputSurface)
    {
        m_maxSrcRect.right  = MOS_MAX(m_maxSrcRect.right, inputSurface->rcSrc.right);
        m_maxSrcRect.bottom = MOS_MAX(m_maxSrcRect.bottom, inputSurface->rcSrc.bottom);
    }

    // Swap buffers for next iteration
    m_currentDnOutput   = (m_currentDnOutput + 1) & 1;
    m_currentStmmIndex  = (m_currentStmmIndex + 1) & 1;

    return MOS_STATUS_SUCCESS;
}

uint32_t VpResourceManager::GetHistogramSurfaceSize(VP_EXECUTE_CAPS& caps, uint32_t inputWidth, uint32_t inputHeight)
{
    // Allocate Rgb Histogram surface----------------------------------------------
    // Size of RGB histograms, 1 set for each slice. For single slice, other set will be 0
    uint32_t dwSize = VP_VEBOX_RGB_HISTOGRAM_SIZE;
    dwSize += VP_VEBOX_RGB_ACE_HISTOGRAM_SIZE_RESERVED;
    // Size of ACE histograms, 1 set for each slice. For single slice, other set will be 0
    dwSize += VP_VEBOX_ACE_HISTOGRAM_SIZE_PER_FRAME_PER_SLICE *         // Ace histogram size per slice
        VP_NUM_FRAME_PREVIOUS_CURRENT *                                 // Ace for Prev + Curr
        VP_VEBOX_MAX_SLICES;                                            // Total number of slices
    return dwSize;
}

MOS_STATUS VpResourceManager::AssignExecuteResource(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface, VP_SURFACE_GROUP &surfGroup)
{
    surfGroup.clear();

    if (caps.bVebox)
    {
        // Create Vebox Resources
        VP_PUBLIC_CHK_STATUS_RETURN(AssignVeboxResource(caps, inputSurface, outputSurface, surfGroup));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_FORMAT GetVeboxOutputFormat(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat, MOS_FORMAT outputFormat)
{
    if (executeCaps.bIECP)
    {
        // Upsampling to yuv444 for IECP input/output.
        // To align with legacy path, need to check whether inputFormat can also be used for IECP case,
        // in which case IECP down sampling will be applied.
        return Format_AYUV;
    }
    else if (executeCaps.bDI && VpHal_GetSurfaceColorPack(inputFormat) == VPHAL_COLORPACK_420)
    {
        // If the input is 4:2:0, then chroma data is doubled vertically to 4:2:2
        // In legacy path, NV12 will be used if target output is not YUV2 to save bandwidth.
        // Need to check whether it can applied here.
        return Format_YUY2;
    }

    return inputFormat;
}

MOS_STATUS VpResourceManager::ReAllocateVeboxOutputSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface,  bool &allocated)
{
    MOS_RESOURCE_MMC_MODE           surfCompressionMode = MOS_MMC_DISABLED;
    bool                            bSurfCompressible   = false;
    uint32_t                        i                   = 0;

    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface->osSurface);

    MOS_FORMAT format = GetVeboxOutputFormat(caps, inputSurface->osSurface->Format, outputSurface->osSurface->Format);

    allocated = false;
    if (IS_VP_VEBOX_DN_ONLY(caps))
    {
        bSurfCompressible = inputSurface->osSurface->bCompressible;
        surfCompressionMode = inputSurface->osSurface->CompressionMode;
    }
    else
    {
        bSurfCompressible = true;
        surfCompressionMode = MOS_MMC_MC;
    }

    if (m_currentFrameIds.pastFrameAvailable && m_currentFrameIds.futureFrameAvailable)
    {
        // Not switch back to 2 after being set to 4.
        m_veboxOutputCount = 4;
    }

    for (i = 0; i < m_veboxOutputCount; i++)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_veboxOutput[i],
            "VeboxSurfaceOutput",
            format,
            MOS_GFXRES_2D,
            inputSurface->osSurface->TileType,
            inputSurface->osSurface->dwWidth,
            inputSurface->osSurface->dwHeight,
            bSurfCompressible,
            surfCompressionMode,
            allocated,
            false,
            MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF));

        m_veboxOutput[i]->ColorSpace = inputSurface->ColorSpace;
        m_veboxOutput[i]->rcDst      = inputSurface->rcDst;
        m_veboxOutput[i]->rcSrc      = inputSurface->rcSrc;
        m_veboxOutput[i]->rcMaxSrc   = inputSurface->rcMaxSrc;

        // When IECP is enabled and Bob or interlaced scaling is selected for interlaced input,
        // output surface's SampleType should be same to input's. Bob is being
        // done in Composition part
        if ((caps.bIECP &&
            (caps.bDI && caps.bBobDI)) ||
            (caps.bIScaling))
        {
            m_veboxOutput[i]->SampleType = inputSurface->SampleType;
        }
        else
        {
            m_veboxOutput[i]->SampleType = SAMPLE_PROGRESSIVE;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::ReAllocateVeboxDenoiseOutputSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, bool &allocated)
{
    MOS_RESOURCE_MMC_MODE           surfCompressionMode = MOS_MMC_DISABLED;
    bool                            bSurfCompressible   = false;

    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);

    allocated = false;
    if (IS_VP_VEBOX_DN_ONLY(caps))
    {
        bSurfCompressible = inputSurface->osSurface->bCompressible;
        surfCompressionMode = inputSurface->osSurface->CompressionMode;
    }
    else
    {
        bSurfCompressible = true;
        surfCompressionMode = MOS_MMC_MC;
    }

    for (uint32_t i = 0; i < VP_NUM_DN_SURFACES; i++)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_veboxDenoiseOutput[i],
            "VeboxFFDNSurface",
            inputSurface->osSurface->Format,
            MOS_GFXRES_2D,
            inputSurface->osSurface->TileType,
            inputSurface->osSurface->dwWidth,
            inputSurface->osSurface->dwHeight,
            bSurfCompressible,
            surfCompressionMode,
            allocated,
            false,
            MOS_HW_RESOURCE_USAGE_VP_INPUT_REFERENCE_FF));

        // if allocated, pVeboxState->PastSurface is not valid for DN reference.
        if (allocated)
        {
            // If DI is enabled, try to use app's reference if provided
            if (caps.bRefValid && caps.bDI)
            {
                //CopySurfaceValue(pVeboxState->m_previousSurface, pVeboxState->m_currentSurface->pBwdRef);
            }
            else
            {
                caps.bRefValid = false;
            }
            // Report Compress Status
            m_reporting.FFDNCompressible = bSurfCompressible;
            m_reporting.FFDNCompressMode = (uint8_t)(surfCompressionMode);
        }
        else
        {
            caps.bRefValid = true;
        }

        // DN's output format should be same to input
        m_veboxDenoiseOutput[i]->SampleType =
            inputSurface->SampleType;

        // Set Colorspace of FFDN
        m_veboxDenoiseOutput[i]->ColorSpace = inputSurface->ColorSpace;

        // Copy FrameID and parameters, as DN output will be used as next blt's current
        m_veboxDenoiseOutput[i]->FrameID = inputSurface->FrameID;

        // Place Holder to update report for debug purpose
    }
    return MOS_STATUS_SUCCESS;
}

// Allocate STMM (Spatial-Temporal Motion Measure) Surfaces
MOS_STATUS VpResourceManager::ReAllocateVeboxSTMMSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, bool &allocated)
{
    MOS_RESOURCE_MMC_MODE           surfCompressionMode = MOS_MMC_DISABLED;
    bool                            bSurfCompressible   = false;
    uint32_t                        i                   = 0;

    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);

    allocated = false;
    for (i = 0; i < VP_NUM_STMM_SURFACES; i++)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_veboxSTMMSurface[i],
            "VeboxSTMMSurface",
            Format_STMM,
            MOS_GFXRES_2D,
            MOS_TILE_Y,
            inputSurface->osSurface->dwWidth,
            inputSurface->osSurface->dwHeight,
            bSurfCompressible,
            surfCompressionMode,
            allocated,
            true,
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF));

        if (allocated)
        {
            // Report Compress Status
            m_reporting.STMMCompressible = bSurfCompressible;
            m_reporting.STMMCompressMode = (uint8_t)surfCompressionMode;
        }
    }
    return MOS_STATUS_SUCCESS;
}

void VpResourceManager::DestoryVeboxOutputSurface()
{
    for (uint32_t i = 0; i < VP_MAX_NUM_VEBOX_SURFACES; i++)
    {
        m_allocator.DestroyVpSurface(m_veboxOutput[i]);
    }
}

void VpResourceManager::DestoryVeboxDenoiseOutputSurface()
{
    for (uint32_t i = 0; i < VP_NUM_DN_SURFACES; i++)
    {
        m_allocator.DestroyVpSurface(m_veboxDenoiseOutput[i]);
    }
}

void VpResourceManager::DestoryVeboxSTMMSurface()
{
    // Free DI history buffers (STMM = Spatial-temporal motion measure)
    for (uint32_t i = 0; i < VP_NUM_STMM_SURFACES; i++)
    {
        m_allocator.DestroyVpSurface(m_veboxSTMMSurface[i]);
    }
}

uint32_t VpResourceManager::Get3DLutSize()
{
    return VP_VEBOX_HDR_3DLUT65;
}

MOS_STATUS VpResourceManager::AllocateVeboxResource(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface->osSurface);

    MOS_FORMAT                      format;
    MOS_TILE_TYPE                   TileType;
    uint32_t                        dwWidth;
    uint32_t                        dwHeight;
    uint32_t                        dwSize;
    uint32_t                        i;
    MOS_RESOURCE_MMC_MODE           surfCompressionMode = MOS_MMC_DISABLED;
    bool                            bSurfCompressible   = false;
    bool                            bAllocated          = false;

    if (IS_VP_VEBOX_DN_ONLY(caps))
    {
        bSurfCompressible = inputSurface->osSurface->bCompressible;
        surfCompressionMode = inputSurface->osSurface->CompressionMode;
    }
    else
    {
        bSurfCompressible = true;
        surfCompressionMode = MOS_MMC_MC;
    }

    // Decide DN output surface
    if (VeboxOutputNeeded(caps))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(ReAllocateVeboxOutputSurface(caps, inputSurface, outputSurface, bAllocated));
    }
    else
    {
        DestoryVeboxOutputSurface();
    }

    if (VeboxDenoiseOutputNeeded(caps))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(ReAllocateVeboxDenoiseOutputSurface(caps, inputSurface, bAllocated));
        if (bAllocated)
        {
            m_currentDnOutput = 0;
        }
    }
    else
    {
        DestoryVeboxDenoiseOutputSurface();
    }

    if (VeboxSTMMNeeded(caps))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(ReAllocateVeboxSTMMSurface(caps, inputSurface, bAllocated));
        if (bAllocated)
        {
            m_currentStmmIndex = 0;
        }
    }
    else
    {
        DestoryVeboxSTMMSurface();
    }

#if VEBOX_AUTO_DENOISE_SUPPORTED
    // Allocate Temp Surface for Vebox Update kernels----------------------------------------
    // the surface size is one Page
    dwSize = MHW_PAGE_SIZE;
    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_veboxDNTempSurface,
        "VeboxDNTempSurface",
        Format_Buffer,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        dwSize,
        1,
        false,
        MOS_MMC_DISABLED,
        bAllocated,
        true,
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF));

    // Allocate Spatial Attributes Configuration Surface for DN kernel Gen9+-----------
    dwSize = MHW_PAGE_SIZE;
    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_veboxDNSpatialConfigSurface,
        "VeboxSpatialAttributesConfigurationSurface",
        Format_RAW,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        dwSize,
        1,
        false,
        MOS_MMC_DISABLED,
        bAllocated,
        false,
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF));

    if (bAllocated)
    {
        // initialize Spatial Attributes Configuration Surface
        VP_PUBLIC_CHK_STATUS_RETURN(InitVeboxSpatialAttributesConfiguration());
    }

#endif

    dwSize = GetHistogramSurfaceSize(caps, inputSurface->osSurface->dwWidth, inputSurface->osSurface->dwHeight);

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_veboxRgbHistogram,
        "VeboxLaceAceRgbHistogram",
        Format_Buffer,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        dwSize,
        1,
        false,
        MOS_MMC_DISABLED,
        bAllocated,
        false,
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_WRITE_FF));

    // Allocate Statistics State Surface----------------------------------------
    // Width to be a aligned on 64 bytes and height is 1/4 the height
    // Per frame information written twice per frame for 2 slices
    // Surface to be a rectangle aligned with dwWidth to get proper dwSize
    // APG PAth need to make sure input surface width/height is what to processed width/Height
    dwWidth = MOS_ALIGN_CEIL(inputSurface->osSurface->dwWidth, 64);
    dwHeight = MOS_ROUNDUP_DIVIDE(inputSurface->osSurface->dwHeight, 4) +
        MOS_ROUNDUP_DIVIDE(VP_VEBOX_STATISTICS_SIZE * sizeof(uint32_t), dwWidth);

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_veboxStatisticsSurface,
        "VeboxStatisticsSurface",
        Format_Buffer,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        dwWidth,
        dwHeight,
        false,
        MOS_MMC_DISABLED,
        bAllocated,
        true,
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_WRITE_FF));

    if (caps.bHDR3DLUT)
    {
        // HDR
        dwSize = Get3DLutSize();
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_vebox3DLookUpTables,
            "Vebox3DLutTableSurface",
            Format_Buffer,
            MOS_GFXRES_BUFFER,
            MOS_TILE_LINEAR,
            dwSize,
            1,
            false,
            MOS_MMC_DISABLED,
            bAllocated));
    }
    // cappipe

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignVeboxResource(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface, VP_SURFACE_GROUP &surfGroup)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface->osSurface);

    MOS_FORMAT                      format;
    MOS_TILE_TYPE                   TileType;
    uint32_t                        dwWidth;
    uint32_t                        dwHeight;
    uint32_t                        dwSize;
    uint32_t                        i;

    VP_PUBLIC_CHK_STATUS_RETURN(AllocateVeboxResource(caps, inputSurface, outputSurface));

    surfGroup.insert(std::make_pair(SurfaceTypeVeboxInput, inputSurface));
    surfGroup.insert(std::make_pair(SurfaceTypeVeboxCurrentOutput, GetVeboxOutputSurface(caps, outputSurface)));

    if (caps.bDN)
    {
        // Insert DN output surface
        surfGroup.insert(std::make_pair(SurfaceTypeDNOutput, m_veboxDenoiseOutput[m_currentDnOutput]));
        // Insert DN Reference surface
        if (caps.bRefValid)
        {
            surfGroup.insert(std::make_pair(SurfaceTypeVeboxPreviousInput, m_veboxDenoiseOutput[(m_currentDnOutput + 1) & 1]));
        }
    }

    // Insert STMM input surface
    surfGroup.insert(std::make_pair(SurfaceTypeSTMMIn, m_veboxSTMMSurface[m_currentStmmIndex]));
    // Insert STMM output surface
    surfGroup.insert(std::make_pair(SurfaceTypeSTMMOut, m_veboxSTMMSurface[(m_currentStmmIndex + 1) & 1]));

#if VEBOX_AUTO_DENOISE_SUPPORTED
    // Insert Vebox auto DN noise level surface
    surfGroup.insert(std::make_pair(SurfaceTypeAutoDNNoiseLevel, m_veboxDNTempSurface));
    // Insert Vebox auto DN spatial config surface/buffer
    surfGroup.insert(std::make_pair(SurfaceTypeAutoDNSpatialConfig, m_veboxDNSpatialConfigSurface));
#endif

    // Insert Vebox histogram surface
    surfGroup.insert(std::make_pair(SurfaceTypeLaceAceRGBHistogram, m_veboxRgbHistogram));

    // Insert Vebox statistics surface
    surfGroup.insert(std::make_pair(SurfaceTypeStatistics, m_veboxStatisticsSurface));

    return MOS_STATUS_SUCCESS;
}

VP_SURFACE* VpResourceManager::GetVeboxOutputSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *outputSurface)
{
    if (caps.bRender)
    {
        // Place Holder when enable DI
        return nullptr;
    }

    if (!caps.bSFC) // Vebox output directlly to output surface
    {
        // RenderTarget will be assigned in VpVeboxCmdPacket::GetSurface.
        return nullptr;
    }
    else if (caps.bDI && caps.bVebox) // Vebox DI enable
    {
        // Place Holder when enable DI
        return nullptr;
    }
    else if (caps.bIECP) // SFC + IECP enabled, output to internal surface
    {
        return m_veboxOutput[m_currentDnOutput];
    }
    else if (caps.bDN) // SFC + DN case
    {
        // DN + SFC scenario needs IECP implicitly, which need vebox output surface being assigned.
        // Use m_currentDnOutput to ensure m_veboxOutput surface paired with DN output surface.
        return m_veboxOutput[m_currentDnOutput];
    }
    else
    {
        // Write to SFC cases, Vebox output is not needed.
        VP_PUBLIC_NORMALMESSAGE("No need output for Vebox output");
        return nullptr;
    }
}

MOS_STATUS VpResourceManager::InitVeboxSpatialAttributesConfiguration()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_veboxDNSpatialConfigSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_veboxDNSpatialConfigSurface->osSurface);

    uint8_t* data = (uint8_t*)& g_cInit_VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATIONS;
    return m_allocator.Write1DSurface(m_veboxDNSpatialConfigSurface, data,
        (uint32_t)sizeof(VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION));
}

bool VpResourceManager::VeboxOutputNeeded(VP_EXECUTE_CAPS& caps)
{
    // If DN and/or Hotpixel are the only functions enabled then the only output is the Denoised Output
    // and no need vebox output.
    // For any other vebox features being enabled, vebox output surface is needed.
    if (caps.bDI            ||
        caps.bQueryVariance ||
        caps.bIECP          ||
        (caps.bDN && caps.bSFC))  // DN + SFC needs IECP implicitly and outputs to DI surface
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VpResourceManager::VeboxDenoiseOutputNeeded(VP_EXECUTE_CAPS& caps)
{
    return caps.bDN;
}

bool VpResourceManager::VeboxSTMMNeeded(VP_EXECUTE_CAPS& caps)
{
    return caps.bDI || caps.bDN;
}

};