/*
* Copyright (c) 2018-2024, Intel Corporation
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
#include "vp_utils.h"
#include "vp_platform_interface.h"

using namespace std;
namespace vp
{

#define VP_SAME_SAMPLE_THRESHOLD        0
#define VP_COMP_CMFC_COEFF_WIDTH        64
#define VP_COMP_CMFC_COEFF_HEIGHT       8

inline bool IsInterleaveFirstField(VPHAL_SAMPLE_TYPE sampleType)
{
    VP_FUNC_CALL();
    return ((sampleType == SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD)   ||
            (sampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)     ||
            (sampleType == SAMPLE_SINGLE_TOP_FIELD));
}

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

VpResourceManager::VpResourceManager(MOS_INTERFACE &osInterface, VpAllocator &allocator, VphalFeatureReport &reporting, vp::VpPlatformInterface &vpPlatformInterface, MediaCopyWrapper *mediaCopyWrapper, vp::VpUserFeatureControl *vpUserFeatureControl)
    : m_osInterface(osInterface), m_allocator(allocator), m_reporting(reporting), m_vpPlatformInterface(vpPlatformInterface), m_mediaCopyWrapper(mediaCopyWrapper)
{
    InitSurfaceConfigMap();
    m_userSettingPtr = m_osInterface.pfnGetUserSettingInstance(&m_osInterface);
    m_vpUserFeatureControl = vpUserFeatureControl;
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

    if (m_veboxStatisticsSurfacefor1stPassofSfc2Pass)
    {
        m_allocator.DestroyVpSurface(m_veboxStatisticsSurfacefor1stPassofSfc2Pass);
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

    if (m_vebox3DLookUpTables2D)
    {
        m_allocator.DestroyVpSurface(m_vebox3DLookUpTables2D);
    }

    if (m_3DLutKernelCoefSurface)
    {
        m_allocator.DestroyVpSurface(m_3DLutKernelCoefSurface);
    }

    if (m_veboxDnHVSTables)
    {
        m_allocator.DestroyVpSurface(m_veboxDnHVSTables);
    }

    if (m_vebox1DLookUpTables)
    {
        m_allocator.DestroyVpSurface(m_vebox1DLookUpTables);
    }

    if (m_innerTileConvertInput)
    {
        m_allocator.DestroyVpSurface(m_innerTileConvertInput);
    }

    if (m_temperalInput)
    {
        m_allocator.DestroyVpSurface(m_temperalInput);
    }

    if (m_hdrResourceManager)
    {
        MOS_Delete(m_hdrResourceManager);
    }

    while (!m_intermediaSurfaces.empty())
    {
        VP_SURFACE * surf = m_intermediaSurfaces.back();
        m_allocator.DestroyVpSurface(surf);
        m_intermediaSurfaces.pop_back();
    }

    for (int i = 0; i < VP_NUM_FC_INTERMEDIA_SURFACES; ++i)
    {
        m_allocator.DestroyVpSurface(m_fcIntermediateSurface[i]);
    }

    m_allocator.DestroyVpSurface(m_cmfcCoeff);
    m_allocator.DestroyVpSurface(m_decompressionSyncSurface);
    for (int i = 0; i < VP_COMP_MAX_LAYERS; ++i)
    {
        if (m_fcIntermediaSurfaceInput[i])
        {
            m_allocator.DestroyVpSurface(m_fcIntermediaSurfaceInput[i]);
        }
        if (m_fcSeparateIntermediaSurfaceSecPlaneInput[i])
        {
            m_allocator.DestroyVpSurface(m_fcSeparateIntermediaSurfaceSecPlaneInput[i]);
        }
    }
    if (m_fcIntermediaSurfaceOutput)
    {
        m_allocator.DestroyVpSurface(m_fcIntermediaSurfaceOutput);
    }

    m_allocator.CleanRecycler();
}

void VpResourceManager::CleanTempSurfaces()
{
    VP_FUNC_CALL();

    while (!m_tempSurface.empty())
    {
        auto it = m_tempSurface.begin();
        m_allocator.DestroyVpSurface(it->second);
        m_tempSurface.erase(it);
    }
}

MOS_STATUS VpResourceManager::OnNewFrameProcessStart(SwFilterPipe &pipe)
{
    VP_FUNC_CALL();

    MT_LOG1(MT_VP_HAL_ONNEWFRAME_PROC_START, MT_NORMAL, MT_FUNC_START, 1);
    VP_SURFACE *inputSurface    = pipe.GetSurface(true, 0);
    VP_SURFACE *outputSurface   = pipe.GetSurface(false, 0);
    SwFilter   *diFilter        = pipe.GetSwFilter(true, 0, FeatureTypeDi);

    if (nullptr == inputSurface && nullptr == outputSurface)
    {
        VP_PUBLIC_ASSERTMESSAGE("Both input and output surface being nullptr!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    if (0 != m_currentPipeIndex)
    {
        VP_PUBLIC_ASSERTMESSAGE("m_currentPipeIndex(%d) is not 0. May caused by OnNewFrameProcessEnd not paired with OnNewFrameProcessStart!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    VP_SURFACE *pastSurface   = pipe.GetPastSurface(0);
    VP_SURFACE *futureSurface = pipe.GetFutureSurface(0);

    int32_t currentFrameId = inputSurface ? inputSurface->FrameID : outputSurface->FrameID;
    int32_t pastFrameId = pastSurface ? pastSurface->FrameID : 0;
    int32_t futureFrameId = futureSurface ? futureSurface->FrameID : 0;

    m_currentFrameIds.valid                     = true;
    m_currentFrameIds.diEnabled                 = nullptr != diFilter;
    m_currentFrameIds.currentFrameId            = currentFrameId;
    m_currentFrameIds.pastFrameId               = pastFrameId;
    m_currentFrameIds.futureFrameId             = futureFrameId;
    m_currentFrameIds.pastFrameAvailable        = pastSurface ? true : false;
    m_currentFrameIds.futureFrameAvailable      = futureSurface ? true : false;

    // Only set sameSamples flag DI enabled frames.
    if (m_pastFrameIds.valid && m_currentFrameIds.pastFrameAvailable &&
        m_pastFrameIds.diEnabled && m_currentFrameIds.diEnabled && m_isPastFrameVeboxDiUsed)
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

        if (m_sameSamples)
        {
            m_outOfBound = false;
        }
        else
        {
            m_outOfBound =
                OUT_OF_BOUNDS(
                        m_currentFrameIds.pastFrameId - m_pastFrameIds.currentFrameId,
                        -VP_SAME_SAMPLE_THRESHOLD,
                        VP_SAME_SAMPLE_THRESHOLD);
        }
    }
    // bSameSamples flag also needs to be set for no reference case
    else if (m_pastFrameIds.valid && !m_currentFrameIds.pastFrameAvailable &&
        m_pastFrameIds.diEnabled && m_currentFrameIds.diEnabled && m_isPastFrameVeboxDiUsed)
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
    if (!m_sameSamples)
    {
        m_currentDnOutput   = (m_currentDnOutput + 1) & 1;
        m_currentStmmIndex  = (m_currentStmmIndex + 1) & 1;
    }

    m_pastFrameIds = m_currentFrameIds;

    m_isFcIntermediateSurfacePrepared = false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::GetFormatForFcIntermediaSurface(MOS_FORMAT& format,
    MEDIA_CSPACE &colorSpace, SwFilterPipe &featurePipe)
{
    VP_FUNC_CALL();

    PVP_SURFACE                     target = nullptr;
    int32_t                         surfCount = (int32_t)featurePipe.GetSurfaceCount(true);
    PVP_SURFACE                     src = nullptr;
    int32_t                         i = 0, j = 0;
    int32_t                         csc_count = 0;
    int32_t                         csc_min = surfCount + 1;
    int32_t                         cspace_in_use[CSpace_Count] = {};
    bool                            bYUVTarget = false;
    MEDIA_CSPACE                    cs = CSpace_Any;
    MEDIA_CSPACE                    tempColorSpace = CSpace_Any;
    MEDIA_CSPACE                    mainColorSpace = CSpace_None;

    auto PostProcess = [&](MOS_STATUS status)
    {
        VP_PUBLIC_NORMALMESSAGE("Main_ColorSpace %d, Temp_ColorSpace %d, csc_count %d.",
            mainColorSpace, tempColorSpace, csc_count);
        colorSpace = tempColorSpace;
        // Set AYUV or ARGB output depending on intermediate cspace
        if (KernelDll_IsCspace(colorSpace, CSpace_RGB))
        {
            format = Format_A8R8G8B8;
        }
        else
        {
            format = Format_AYUV;
        }
        return status;
    };

    // Check if target is YUV
    target     = featurePipe.GetSurface(false, 0);

    VP_PUBLIC_CHK_NULL_RETURN(target);
    VP_PUBLIC_CHK_NULL_RETURN(target->osSurface);

    bYUVTarget = IS_RGB_FORMAT(target->osSurface->Format) ? false : true;

    // Gets primary video cspace
    // Implements xvYCC passthrough mode
    // Set Color Spaces in use
    MOS_ZeroMemory(cspace_in_use, sizeof(cspace_in_use));
    for (i = 0; i < surfCount; i++)
    {
        // Get current source
        src = featurePipe.GetSurface(true, i);
        VP_PUBLIC_CHK_NULL_RETURN(src);
        VP_PUBLIC_CHK_NULL_RETURN(src->osSurface);

        // Save Main Video color space
        if (src->SurfType == SURF_IN_PRIMARY &&
            mainColorSpace == CSpace_None)
        {
            mainColorSpace = src->ColorSpace;
        }

        // Set xvYCC pass through mode
        if (bYUVTarget &&
            (src->ColorSpace == CSpace_xvYCC709 ||
             src->ColorSpace == CSpace_xvYCC601))
        {
            tempColorSpace = src->ColorSpace;
            return PostProcess(MOS_STATUS_SUCCESS);
        }

        // Don't take PAL formats into consideration
        if ((!IS_PAL_FORMAT(src->osSurface->Format)) &&
             src->ColorSpace > CSpace_Any &&
             src->ColorSpace < CSpace_Count)
        {
            cs = KernelDll_TranslateCspace(src->ColorSpace);
            if (cs >= CSpace_Any)
            {
                cspace_in_use[cs]++;
            }
        }
    }

    // For every CS in use, iterate through source CS and keep a
    // count of number of CSC operation needed. Determine the Temporary
    // color space as the one requiring min. # of CSC ops.
    for (j = (CSpace_Any + 1); j < CSpace_Count; j++)
    {
        // Skip color spaces not in use
        if (!cspace_in_use[j])
        {
            continue;
        }

        // Count # of CS conversions
        cs = (MEDIA_CSPACE) j;
        csc_count = 0;
        for (i = 0; i < surfCount; i++)
        {
            // Get current source
            src = featurePipe.GetSurface(true, i);
            VP_PUBLIC_CHK_NULL_RETURN(src);
            VP_PUBLIC_CHK_NULL_RETURN(src->osSurface);

            auto featureSubPipe = featurePipe.GetSwFilterSubPipe(true, i);
            VP_PUBLIC_CHK_NULL_RETURN(featureSubPipe);

            // Ignore palletized layers
            if (IS_PAL_FORMAT(src->osSurface->Format) ||
                src->ColorSpace == CSpace_Any)
            {
                continue;
            }

            auto procamp = dynamic_cast<SwFilterProcamp *>(featureSubPipe->GetSwFilter(FeatureTypeProcamp));
            // Check if CSC/PA is required
            if (KernelDll_TranslateCspace(src->ColorSpace) != cs ||
                (procamp &&
                 procamp->GetSwFilterParams().procampParams &&
                 procamp->GetSwFilterParams().procampParams->bEnabled))
            {
                csc_count++;
            }
        }

        // Save best choice as requiring minimum number of CSC operations
        // Use main cspace as default if same CSC count
        if ((csc_count <  csc_min) ||
            (csc_count == csc_min && cs == mainColorSpace))
        {
            tempColorSpace = cs;
            csc_min = csc_count;
        }
    }

    // If all layers are palletized, use the CS from first layer (as good as any other)
    if (tempColorSpace == CSpace_Any && surfCount > 0)
    {
        src = featurePipe.GetSurface(true, 0);
        VP_PUBLIC_CHK_NULL_RETURN(src);
        tempColorSpace = src->ColorSpace;
    }

    return PostProcess(MOS_STATUS_SUCCESS);
}

MOS_STATUS VpResourceManager::PrepareFcIntermediateSurface(SwFilterPipe &featurePipe)
{
    VP_FUNC_CALL();

    if (m_isFcIntermediateSurfacePrepared)
    {
        return MOS_STATUS_SUCCESS;
    }

    m_isFcIntermediateSurfacePrepared = true;

    MOS_FORMAT      format      = Format_Any;
    MEDIA_CSPACE    colorSpace  = CSpace_Any;

    VP_PUBLIC_CHK_STATUS_RETURN(GetFormatForFcIntermediaSurface(format, colorSpace, featurePipe));

    auto target = featurePipe.GetSurface(false, 0);
    VP_PUBLIC_CHK_NULL_RETURN(target);
    VP_PUBLIC_CHK_NULL_RETURN(target->osSurface);

    uint32_t tempWidth  = target->osSurface->dwWidth;
    uint32_t tempHeight = target->osSurface->dwHeight;

    uint32_t curWidth = 0;
    uint32_t curHeight = 0;

    if (m_fcIntermediateSurface[0])
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_fcIntermediateSurface[0]->osSurface);
        curWidth    = m_fcIntermediateSurface[0]->osSurface->dwWidth;
        curHeight   = m_fcIntermediateSurface[0]->osSurface->dwHeight;
    }

    // Allocate buffer in fixed increments
    tempWidth  = MOS_ALIGN_CEIL(tempWidth , VPHAL_BUFFER_SIZE_INCREMENT);
    tempHeight = MOS_ALIGN_CEIL(tempHeight, VPHAL_BUFFER_SIZE_INCREMENT);

    for (int i = 0; i < VP_NUM_FC_INTERMEDIA_SURFACES; ++i)
    {
        if (tempWidth > curWidth || tempHeight > curHeight)
        {
            bool allocated = false;
            // Get surface parameter.
            // Use A8R8G8B8 instead of real surface format to ensure the surface can be reused for both AYUV and A8R8G8B8,
            // since for A8R8G8B8, tile64 is used, while for AYUV, both tile4 and tile64 is ok.
            if (m_fcIntermediateSurface[i] && m_fcIntermediateSurface[i]->osSurface)
            {
                m_fcIntermediateSurface[i]->osSurface->Format = Format_A8R8G8B8;
            }
            VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
                m_fcIntermediateSurface[i],
                "fcIntermediaSurface",
                Format_A8R8G8B8,
                MOS_GFXRES_2D,
                MOS_TILE_Y,
                tempWidth,
                tempHeight,
                false,
                MOS_MMC_DISABLED,
                allocated,
                false,
                IsDeferredResourceDestroyNeeded(),
                MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER,
                MOS_TILE_UNSET_GMM,
                MOS_MEMPOOL_DEVICEMEMORY,
                true));
            m_fcIntermediateSurface[i]->osSurface->Format = format;
            m_fcIntermediateSurface[i]->ColorSpace = colorSpace;
        }
        else
        {
            m_fcIntermediateSurface[i]->osSurface->dwWidth = tempWidth;
            m_fcIntermediateSurface[i]->osSurface->dwHeight = tempHeight;
            m_fcIntermediateSurface[i]->osSurface->Format = format;
            m_fcIntermediateSurface[i]->ColorSpace = colorSpace;
        }
        m_fcIntermediateSurface[i]->rcSrc = target->rcSrc;
        m_fcIntermediateSurface[i]->rcDst = target->rcDst;
    }

    return MOS_STATUS_SUCCESS;
}

void VpResourceManager::OnNewFrameProcessEnd()
{
    VP_FUNC_CALL();
    MT_LOG1(MT_VP_HAL_ONNEWFRAME_PROC_END, MT_NORMAL, MT_FUNC_END, 1);
    m_allocator.CleanRecycler();
    m_currentPipeIndex = 0;
    CleanTempSurfaces();
}

void VpResourceManager::InitSurfaceConfigMap()
{
    VP_FUNC_CALL();
    ///*             _b64DI
    //               |      _sfcEnable
    //               |      |      _sameSample
    //               |      |      |      _outOfBound
    //               |      |      |      |      _pastRefAvailable
    //               |      |      |      |      |      _futureRefAvailable
    //               |      |      |      |      |      |      _firstDiField
    //               |      |      |      |      |      |      |      _currentInputSurface
    //               |      |      |      |      |      |      |      |                     _pastInputSurface
    //               |      |      |      |      |      |      |      |                     |                       _currentOutputSurface
    //               |      |      |      |      |      |      |      |                     |                       |                     _pastOutputSurface*/
    //               |      |      |      |      |      |      |      |                     |                       |                     |                 */
    // sfc Enable
    AddSurfaceConfig(true,  true,  false, false, true,  false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_FRAME0);
    AddSurfaceConfig(true,  true,  true,  false, true,  false, false, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL,     VEBOX_SURFACE_NULL,   VEBOX_SURFACE_NULL);
    AddSurfaceConfig(true,  true,  false, false, false, false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(true,  true,  false, false, false, false, false, VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(true,  true,  false, false, true,  false, false, VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_FRAME0);
    AddSurfaceConfig(true,  true,  true,  false, false, false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(true,  true,  true,  false, false, false, false, VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(true,  true,  true,  false, true,  false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_FRAME0);

    // outOfBound
    AddSurfaceConfig(true,  true,  false, true,  true,  false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_FRAME0);
    AddSurfaceConfig(true,  true,  false, true,  true,  false, false, VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL);

    // sfc disable
    AddSurfaceConfig(true,  false,  false, false, true,  false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_OUTPUT);
    AddSurfaceConfig(true,  false,  true,  false, true,  false, false, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL,     VEBOX_SURFACE_NULL,   VEBOX_SURFACE_NULL);
    AddSurfaceConfig(true,  false,  false, false, false, false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_OUTPUT, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(true,  false,  false, false, false, false, false, VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(true,  false,  false, false, true,  false, false, VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_FRAME0);
    AddSurfaceConfig(true,  false,  true,  false, false, false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_OUTPUT, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(true,  false,  true,  false, false, false, false, VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL);

    //30i -> 30p sfc Enable
    AddSurfaceConfig(false, true,   false, false, false, false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(false, true,   false, false, true,  false, false, VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_FRAME0);
    AddSurfaceConfig(false, true,   false, false, true,  false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_FRAME0);
    AddSurfaceConfig(false, true,   false, true,  true,  false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_FRAME0);
    AddSurfaceConfig(false, true,   false, true,  true,  false, false, VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_PAST_REF, VEBOX_SURFACE_FRAME1, VEBOX_SURFACE_FRAME0);
    
    //30i -> 30p sfc disable
    AddSurfaceConfig(false, false,  false, false, true,  false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_OUTPUT, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(false, false,  false, true,  true,  false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_OUTPUT, VEBOX_SURFACE_NULL);
    AddSurfaceConfig(false, false,  false, false, false, false, true,  VEBOX_SURFACE_INPUT,  VEBOX_SURFACE_NULL,     VEBOX_SURFACE_OUTPUT, VEBOX_SURFACE_NULL);
}

uint32_t VpResourceManager::GetHistogramSurfaceSize(VP_EXECUTE_CAPS& caps, uint32_t inputWidth, uint32_t inputHeight)
{
    VP_FUNC_CALL();

    // Allocate Rgb Histogram surface----------------------------------------------
    // Size of RGB histograms, 1 set for each slice. For single slice, other set will be 0
    uint32_t dwSize = VP_VEBOX_RGB_HISTOGRAM_SIZE;
    dwSize += VP_VEBOX_RGB_ACE_HISTOGRAM_SIZE_RESERVED;
    // Size of ACE histograms, 1 set for each slice. For single slice, other set will be 0
    dwSize += VP_VEBOX_ACE_HISTOGRAM_SIZE_PER_FRAME_PER_SLICE *         // Ace histogram size per slice
        VP_NUM_FRAME_PREVIOUS_CURRENT *                                 // Ace for Prev + Curr
        VP_VEBOX_HISTOGRAM_SLICES_COUNT;                                // Total number of slices
    return dwSize;
}

MOS_STATUS VpResourceManager::GetResourceHint(std::vector<FeatureType> &featurePool, SwFilterPipe& executedFilters, RESOURCE_ASSIGNMENT_HINT &hint)
{
    VP_FUNC_CALL();

    uint32_t    index      = 0;
    SwFilterSubPipe *inputPipe = executedFilters.GetSwFilterSubPipe(true, index);

    // only process Primary surface
    if (inputPipe == nullptr)
    {
        VP_PUBLIC_NORMALMESSAGE("No inputPipe, so there is no hint message!");
        return MOS_STATUS_SUCCESS;
    }
    for (auto filterID : featurePool)
    {
        SwFilter* feature = (SwFilter*)inputPipe->GetSwFilter(FeatureType(filterID));
        if (feature)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(feature->SetResourceAssignmentHint(hint));
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::GetIntermediaColorAndFormat3DLutOutput(VPHAL_CSPACE &colorSpace, MOS_FORMAT &format, SwFilterPipe &executedFilters)
{
    SwFilterHdr *hdr = dynamic_cast<SwFilterHdr *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeHdr));
    if (hdr)
    {
        colorSpace = hdr->GetSwFilterParams().dstColorSpace;
        format     = hdr->GetSwFilterParams().formatOutput;
    }
    else
    {   // caps.b3DlutOutput =1, in hdr tests hdr flag should not be false.
        VP_PUBLIC_ASSERTMESSAGE("It is unexcepted for HDR case with caps.b3DlutOutput as true, return INVALID_PARAMETER");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);

    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::GetIntermediaColorAndFormatBT2020toRGB(VP_EXECUTE_CAPS &caps, VPHAL_CSPACE &colorSpace, MOS_FORMAT &format, SwFilterPipe &executedFilters)
{
    SwFilterCsc *cscOnSfc = dynamic_cast<SwFilterCsc *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeCscOnSfc));
    SwFilterCgc *cgc      = dynamic_cast<SwFilterCgc *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeCgc));

    if (caps.bSFC && nullptr == cscOnSfc)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    if (cscOnSfc)
    {
        colorSpace = cscOnSfc->GetSwFilterParams().output.colorSpace;
        format     = cscOnSfc->GetSwFilterParams().formatOutput;
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(cgc);
        colorSpace = cgc->GetSwFilterParams().dstColorSpace;
        format     = cgc->GetSwFilterParams().formatOutput;
    }

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS VpResourceManager::GetIntermediaOutputSurfaceColorAndFormat(VP_EXECUTE_CAPS &caps, SwFilterPipe &executedFilters, MOS_FORMAT &format, VPHAL_CSPACE &colorSpace)
{
    VP_SURFACE *inputSurface = executedFilters.GetSurface(true, 0);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    if (caps.bRender)
    {
        SwFilterCsc *csc = dynamic_cast<SwFilterCsc *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeCscOnRender));
        if (csc)
        {
            format            = csc->GetSwFilterParams().formatOutput;
            colorSpace        = csc->GetSwFilterParams().output.colorSpace;
            return MOS_STATUS_SUCCESS;
        }

    }
    else if (caps.bSFC)
    {
        SwFilterCsc *csc = dynamic_cast<SwFilterCsc *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeCscOnSfc));
        if (csc)
        {
            format            = csc->GetSwFilterParams().formatOutput;
            colorSpace        = csc->GetSwFilterParams().output.colorSpace;
            return MOS_STATUS_SUCCESS;
        }
    }
    else if (caps.b3DlutOutput)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(GetIntermediaColorAndFormat3DLutOutput(colorSpace, format, executedFilters));
        return MOS_STATUS_SUCCESS;
    }
    else if (caps.bBt2020ToRGB)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(GetIntermediaColorAndFormatBT2020toRGB(caps, colorSpace, format, executedFilters));
        return MOS_STATUS_SUCCESS;
    }
    else if (caps.bVebox)
    {
        SwFilterCsc *csc = dynamic_cast<SwFilterCsc *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeCscOnVebox));
        if (csc)
        {
            format            = csc->GetSwFilterParams().formatOutput;
            colorSpace        = csc->GetSwFilterParams().output.colorSpace;
            return MOS_STATUS_SUCCESS;
        }
    }

    format            = inputSurface->osSurface->Format;
    colorSpace        = inputSurface->ColorSpace;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::GetIntermediaOutputSurfaceParams(VP_EXECUTE_CAPS& caps, VP_SURFACE_PARAMS &params, SwFilterPipe &executedFilters)
{
    VP_FUNC_CALL();

    SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeScaling));
    SwFilterRotMir *rotMir = dynamic_cast<SwFilterRotMir *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeRotMir));
    SwFilterDeinterlace *di = dynamic_cast<SwFilterDeinterlace *>(executedFilters.GetSwFilter(true, 0, FeatureType::FeatureTypeDi));
    VP_SURFACE *inputSurface = executedFilters.GetSurface(true, 0);

    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);

    if (scaling)
    {
        params.width = scaling->GetSwFilterParams().output.dwWidth;
        params.height = scaling->GetSwFilterParams().output.dwHeight;
        params.sampleType = scaling->GetSwFilterParams().output.sampleType;
        params.rcSrc = scaling->GetSwFilterParams().output.rcSrc;
        params.rcDst = scaling->GetSwFilterParams().output.rcDst;
        params.rcMaxSrc = scaling->GetSwFilterParams().output.rcMaxSrc;

        if (scaling->GetSwFilterParams().interlacedScalingType == ISCALING_INTERLEAVED_TO_FIELD)
        {
            params.height = scaling->GetSwFilterParams().output.dwHeight / 2;
            params.rcDst.bottom = params.rcDst.bottom / 2;
            params.rcMaxSrc.bottom = params.rcMaxSrc.bottom / 2;
        }
    }
    else
    {
        params.width = inputSurface->osSurface->dwWidth;
        params.height = inputSurface->osSurface->dwHeight;
        params.sampleType = di ? SAMPLE_PROGRESSIVE : inputSurface->SampleType;
        params.rcSrc = inputSurface->rcSrc;
        params.rcDst = inputSurface->rcDst;
        params.rcMaxSrc = inputSurface->rcMaxSrc;
    }

    // Do not use rotatoin flag in scaling swfilter as it has not been initialized here.
    // It will be initialized during pipe update after resource being assigned.
    if (rotMir &&
        (rotMir->GetSwFilterParams().rotation == VPHAL_ROTATION_90 ||
        rotMir->GetSwFilterParams().rotation == VPHAL_ROTATION_270 ||
        rotMir->GetSwFilterParams().rotation == VPHAL_ROTATE_90_MIRROR_VERTICAL ||
        rotMir->GetSwFilterParams().rotation == VPHAL_ROTATE_90_MIRROR_HORIZONTAL))
    {
        swap(params.width, params.height);
        RECT tmp = params.rcSrc;
        RECT_ROTATE(params.rcSrc, tmp);
        tmp = params.rcDst;
        RECT_ROTATE(params.rcDst, tmp);
        tmp = params.rcMaxSrc;
        RECT_ROTATE(params.rcMaxSrc, tmp);
    }

    VP_PUBLIC_CHK_STATUS_RETURN(GetIntermediaOutputSurfaceColorAndFormat(caps, executedFilters, params.format, params.colorSpace));

    params.tileType = MOS_TILE_Y;

    if (SAMPLE_PROGRESSIVE == params.sampleType)
    {
        params.surfCompressionMode = caps.bRender ? MOS_MMC_RC : MOS_MMC_MC;
        params.surfCompressible    = true;
    }
    else
    {
        // MMC does not support interleaved surface.
        VP_PUBLIC_NORMALMESSAGE("Disable MMC for interleaved intermedia surface, sampleType = %d.", params.sampleType);
        params.surfCompressionMode = MOS_MMC_DISABLED;
        params.surfCompressible    = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::GetFcIntermediateSurfaceForOutput(VP_SURFACE *&intermediaSurface, SwFilterPipe &executedFilters)
{
    VP_FUNC_CALL();

    if (!m_isFcIntermediateSurfacePrepared)
    {
        VP_PUBLIC_ASSERTMESSAGE("Fc intermediate surface is not allocated.");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    intermediaSurface = nullptr;

    for (uint32_t i = 0; i < executedFilters.GetSurfaceCount(true); ++i)
    {
        uint32_t j = 0;
        auto surf = executedFilters.GetSurface(true, i);
        VP_PUBLIC_CHK_NULL_RETURN(surf);
        for (j = 0; j < VP_NUM_FC_INTERMEDIA_SURFACES; ++j)
        {
            auto intermediateSurface = m_fcIntermediateSurface[j];
            VP_PUBLIC_CHK_NULL_RETURN(intermediateSurface);
            if (surf->GetAllocationHandle(&m_osInterface) == intermediateSurface->GetAllocationHandle(&m_osInterface))
            {
                uint32_t selIndex = (j + 1) % VP_NUM_FC_INTERMEDIA_SURFACES;
                intermediaSurface = m_fcIntermediateSurface[selIndex];
                break;
            }
        }
        if (j < VP_NUM_FC_INTERMEDIA_SURFACES)
        {
            break;
        }
    }
    // If intermediate surface not in use in current pipe, use first one by default.
    if (nullptr == intermediaSurface)
    {
        intermediaSurface = m_fcIntermediateSurface[0];
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignIntermediaSurface(VP_EXECUTE_CAPS& caps, SwFilterPipe &executedFilters)
{
    VP_FUNC_CALL();

    VP_SURFACE *outputSurface = executedFilters.GetSurface(false, 0);
    VP_SURFACE *intermediaSurface = nullptr;
    VP_SURFACE_PARAMS params            = {};
    if (outputSurface)
    {
        // No need intermedia surface.
        return MOS_STATUS_SUCCESS;
    }

    if (caps.bComposite)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(GetFcIntermediateSurfaceForOutput(intermediaSurface, executedFilters));
    }
    else
    {
        while (m_currentPipeIndex >= m_intermediaSurfaces.size())
        {
            m_intermediaSurfaces.push_back(nullptr);
        }
        bool allocated = false;
        // Get surface parameter.
        GetIntermediaOutputSurfaceParams(caps, params, executedFilters);

        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_intermediaSurfaces[m_currentPipeIndex],
            "IntermediaSurface",
            params.format,
            MOS_GFXRES_2D,
            params.tileType,
            params.width,
            params.height,
            params.surfCompressible,
            params.surfCompressionMode,
            allocated,
            false,
            IsDeferredResourceDestroyNeeded(),
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER));

        VP_PUBLIC_CHK_NULL_RETURN(m_intermediaSurfaces[m_currentPipeIndex]);

        m_intermediaSurfaces[m_currentPipeIndex]->ColorSpace = params.colorSpace;
        m_intermediaSurfaces[m_currentPipeIndex]->rcDst      = params.rcDst;
        m_intermediaSurfaces[m_currentPipeIndex]->rcSrc      = params.rcSrc;
        m_intermediaSurfaces[m_currentPipeIndex]->rcMaxSrc   = params.rcMaxSrc;
        m_intermediaSurfaces[m_currentPipeIndex]->SampleType = params.sampleType;

        intermediaSurface = m_intermediaSurfaces[m_currentPipeIndex];
    }

    VP_PUBLIC_CHK_NULL_RETURN(intermediaSurface);
    VP_SURFACE *output = m_allocator.AllocateVpSurface(*intermediaSurface);
    VP_PUBLIC_CHK_NULL_RETURN(output);
    output->SurfType = SURF_OUT_RENDERTARGET;

    executedFilters.AddSurface(output, false, 0);

    return MOS_STATUS_SUCCESS;
}

VP_SURFACE * VpResourceManager::GetCopyInstOfExtSurface(VP_SURFACE* surf)
{
    VP_FUNC_CALL();

    if (nullptr == surf || 0 == surf->GetAllocationHandle(&m_osInterface))
    {
        return nullptr;
    }
    // Do not use allocation handle as key as some parameters in VP_SURFACE
    // may be different for same allocation, e.g. SurfType for intermedia surface.
    auto it = m_tempSurface.find((uint64_t)surf);
    if (it != m_tempSurface.end())
    {
        return it->second;
    }
    VP_SURFACE *surface = m_allocator.AllocateVpSurface(*surf);
    if (surface)
    {
        m_tempSurface.insert(make_pair((uint64_t)surf, surface));
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Allocate temp surface faild!");
    }

    return surface;
}

MOS_STATUS VpResourceManager::AssignFcResources(VP_EXECUTE_CAPS &caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface,
    std::vector<VP_SURFACE *> &pastSurfaces, std::vector<VP_SURFACE *> &futureSurfaces,
    RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting)
{
    VP_FUNC_CALL();

    bool allocated = false;
    auto *skuTable = m_osInterface.pfnGetSkuTable(&m_osInterface);
    Mos_MemPool memTypeSurfVideoMem = MOS_MEMPOOL_VIDEOMEMORY;

    if (skuTable && MEDIA_IS_SKU(skuTable, FtrLimitedLMemBar))
    {
        memTypeSurfVideoMem = MOS_MEMPOOL_DEVICEMEMORY;
    }

    if (caps.bTemperalInputInuse)
    {
        if (inputSurfaces.size() > 1)
        {
            VP_PUBLIC_ASSERTMESSAGE("Temperal input only has 1 layer, do not support multi-layer case!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        surfSetting.surfGroup.insert(std::make_pair((SurfaceType)(SurfaceTypeFcInputLayer0), m_temperalInput));
    }
    else
    {
        for (size_t i = 0; i < inputSurfaces.size(); ++i)
        {
            surfSetting.surfGroup.insert(std::make_pair((SurfaceType)(SurfaceTypeFcInputLayer0 + i), inputSurfaces[i]));

            if (!resHint.isIScalingTypeNone)
            {
                // For Interlaced scaling, 2nd field is part of the same frame.
                // For Field weaving, 2nd field is passed in as a ref.
                VP_SURFACE *surfField1Dual = nullptr;
                if (resHint.isFieldWeaving)
                {
                    surfField1Dual = pastSurfaces[i];
                    VP_PUBLIC_NORMALMESSAGE("Field weaving case. 2nd field is passed in as a ref.");
                }
                else
                {
                    surfField1Dual = GetCopyInstOfExtSurface(inputSurfaces[i]);
                    VP_PUBLIC_NORMALMESSAGE("Interlaced scaling. 2nd field is part of the same frame.");
                }
                VP_PUBLIC_CHK_NULL_RETURN(surfField1Dual);
                surfSetting.surfGroup.insert(std::make_pair((SurfaceType)(SurfaceTypeFcInputLayer0Field1Dual + i), surfField1Dual));
            }
        }
    }
    surfSetting.surfGroup.insert(std::make_pair(SurfaceTypeFcTarget0, outputSurface));

    // Allocate auto CSC Coeff Surface
    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_cmfcCoeff,
        "CSCCoeffSurface",
        Format_L8,
        MOS_GFXRES_2D,
        MOS_TILE_LINEAR,
        VP_COMP_CMFC_COEFF_WIDTH,
        VP_COMP_CMFC_COEFF_HEIGHT,
        false,
        MOS_MMC_DISABLED,
        allocated,
        false,
        IsDeferredResourceDestroyNeeded(),
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER,
        MOS_TILE_UNSET_GMM,
        memTypeSurfVideoMem,
        VPP_INTER_RESOURCE_NOTLOCKABLE));

    surfSetting.surfGroup.insert(std::make_pair(SurfaceTypeFcCscCoeff, m_cmfcCoeff));

    //for decompreesion sync on interlace input of FC
    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_decompressionSyncSurface,
        "AuxDecompressSyncSurface",
        Format_Buffer,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        32,
        1,
        false,
        MOS_MMC_DISABLED,
        allocated));
    surfSetting.surfGroup.insert(std::make_pair(SurfaceTypeDecompressionSync, m_decompressionSyncSurface));

    // Allocate OCL FC intermedia inputSurface
    for (uint32_t i = 0; i < inputSurfaces.size(); ++i)
    {
        VP_PUBLIC_CHK_NULL_RETURN(inputSurfaces[i]);
        VP_PUBLIC_CHK_NULL_RETURN(inputSurfaces[i]->osSurface);
        MOS_FORMAT fcIntermediaInputFormat                 = Format_Any;
        MOS_FORMAT fcSeparateIntermediaSecPlaneInputFormat = Format_Any;
        uint32_t   widthPL1Factor                          = 1;
        uint32_t   heightPL1Factor                         = 1;
        switch (inputSurfaces[i]->osSurface->Format)
        {
        case Format_RGBP:
        case Format_BGRP:
            fcIntermediaInputFormat = Format_A8R8G8B8;
            break;
        case Format_444P:
            fcIntermediaInputFormat = Format_AYUV;
            break;
        case Format_I420:
        case Format_YV12:
        case Format_IYUV:
        case Format_IMC3:
            fcIntermediaInputFormat = Format_NV12;
            break;
        case Format_422H:
            fcIntermediaInputFormat                 = Format_R8UN;
            fcSeparateIntermediaSecPlaneInputFormat = Format_R8G8UN;
            widthPL1Factor                          = 2;
            break;
        case Format_422V:
            fcIntermediaInputFormat                 = Format_R8UN;
            fcSeparateIntermediaSecPlaneInputFormat = Format_R8G8UN;
            heightPL1Factor                         = 2;
            break;
        case Format_411P:
            fcIntermediaInputFormat                 = Format_R8UN;
            fcSeparateIntermediaSecPlaneInputFormat = Format_R8G8UN;
            widthPL1Factor                          = 4;
            break;
        default:
            break;
        }
        if (fcIntermediaInputFormat != Format_Any)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
                m_fcIntermediaSurfaceInput[i],
                "fcIntermediaSurfaceInput",
                fcIntermediaInputFormat,
                MOS_GFXRES_2D,
                MOS_TILE_Y,
                inputSurfaces[i]->osSurface->dwWidth,
                inputSurfaces[i]->osSurface->dwHeight,
                false,
                MOS_MMC_DISABLED,
                allocated,
                false,
                IsDeferredResourceDestroyNeeded(),
                MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER));

            m_fcIntermediaSurfaceInput[i]->rcSrc         = inputSurfaces[i]->rcSrc;
            m_fcIntermediaSurfaceInput[i]->rcDst         = inputSurfaces[i]->rcDst;
            m_fcIntermediaSurfaceInput[i]->SampleType    = inputSurfaces[i]->SampleType;
            surfSetting.surfGroup.insert(std::make_pair((SurfaceType)(SurfaceTypeFcIntermediaInput + i), m_fcIntermediaSurfaceInput[i]));
        }
        if (fcSeparateIntermediaSecPlaneInputFormat != Format_Any)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
                m_fcSeparateIntermediaSurfaceSecPlaneInput[i],
                "fcSeparateIntermediaSurfaceSecPlaneInput",
                fcSeparateIntermediaSecPlaneInputFormat,
                MOS_GFXRES_2D,
                MOS_TILE_Y,
                inputSurfaces[i]->osSurface->dwWidth / widthPL1Factor,
                inputSurfaces[i]->osSurface->dwHeight / heightPL1Factor,
                false,
                MOS_MMC_DISABLED,
                allocated,
                false,
                IsDeferredResourceDestroyNeeded(),
                MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER));

            m_fcSeparateIntermediaSurfaceSecPlaneInput[i]->rcSrc.top    = inputSurfaces[i]->rcSrc.top / heightPL1Factor;
            m_fcSeparateIntermediaSurfaceSecPlaneInput[i]->rcSrc.bottom = inputSurfaces[i]->rcSrc.bottom / heightPL1Factor;
            m_fcSeparateIntermediaSurfaceSecPlaneInput[i]->rcSrc.left   = inputSurfaces[i]->rcSrc.left / widthPL1Factor;
            m_fcSeparateIntermediaSurfaceSecPlaneInput[i]->rcSrc.right  = inputSurfaces[i]->rcSrc.right / widthPL1Factor;
            m_fcSeparateIntermediaSurfaceSecPlaneInput[i]->rcDst        = inputSurfaces[i]->rcDst;
            m_fcSeparateIntermediaSurfaceSecPlaneInput[i]->SampleType   = inputSurfaces[i]->SampleType;
            surfSetting.surfGroup.insert(std::make_pair((SurfaceType)(SurfaceTypeFcSeparateIntermediaInputSecPlane + i), m_fcSeparateIntermediaSurfaceSecPlaneInput[i]));
        }
    }
    // Allocate OCL FC intermedia outputSurface
    {
        MOS_FORMAT fcIntermediaSurfaceOutputFormat = Format_Any;
        VP_PUBLIC_CHK_NULL_RETURN(outputSurface);
        VP_PUBLIC_CHK_NULL_RETURN(outputSurface->osSurface);
        switch (outputSurface->osSurface->Format)
        {
        case Format_RGBP:
        case Format_BGRP:
            fcIntermediaSurfaceOutputFormat = Format_A8R8G8B8;
            break;
        case Format_444P:
            fcIntermediaSurfaceOutputFormat = Format_AYUV;
            break;
        case Format_I420:
        case Format_IMC3:
        case Format_YV12:
        case Format_IYUV:
            fcIntermediaSurfaceOutputFormat = Format_NV12;
            break;
        default:
            break;
        }
        if (fcIntermediaSurfaceOutputFormat != Format_Any)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
                m_fcIntermediaSurfaceOutput,
                "fcIntermediaSurfaceOutput",
                fcIntermediaSurfaceOutputFormat,
                MOS_GFXRES_2D,
                MOS_TILE_Y,
                outputSurface->osSurface->dwWidth,
                outputSurface->osSurface->dwHeight,
                false,
                MOS_MMC_DISABLED,
                allocated,
                false,
                IsDeferredResourceDestroyNeeded(),
                MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER));
            m_fcIntermediaSurfaceOutput->rcSrc      = outputSurface->rcSrc;
            m_fcIntermediaSurfaceOutput->rcDst      = outputSurface->rcDst;
            m_fcIntermediaSurfaceOutput->SampleType = outputSurface->SampleType;
            surfSetting.surfGroup.insert(std::make_pair((SurfaceType)(SurfaceTypeFcIntermediaOutput), m_fcIntermediaSurfaceOutput));
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignRenderResource(VP_EXECUTE_CAPS &caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface,
    std::vector<VP_SURFACE *> &pastSurfaces, std::vector<VP_SURFACE *> &futureSurfaces, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting, SwFilterPipe& executedFilters)
{
    VP_FUNC_CALL();

    if (caps.bComposite)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AssignFcResources(caps, inputSurfaces, outputSurface, pastSurfaces, futureSurfaces, resHint, surfSetting));
    }
    else if (caps.b3DLutCalc)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(Assign3DLutKernelResource(caps, resHint, surfSetting));
    }
    else if (caps.bHVSCalc)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AssignHVSKernelResource(caps, resHint, surfSetting));
    }
    else if (caps.bRenderHdr)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AssignHdrResource(caps, inputSurfaces, outputSurface, resHint, surfSetting, executedFilters));
    }
    else
    {
        if (1 != inputSurfaces.size())
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        surfSetting.surfGroup.insert(std::make_pair(SurfaceTypeRenderInput, inputSurfaces[0]));
        VP_PUBLIC_CHK_STATUS_RETURN(AssignVeboxResourceForRender(caps, inputSurfaces[0], resHint, surfSetting));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignVeboxResourceForRender(VP_EXECUTE_CAPS &caps, VP_SURFACE *inputSurface, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting)
{
    VP_FUNC_CALL();

    if (!caps.bRender)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignExecuteResource(std::vector<FeatureType> &featurePool, VP_EXECUTE_CAPS& caps, SwFilterPipe &executedFilters)
{
    VP_FUNC_CALL();

    std::vector<VP_SURFACE *> inputSurfaces, pastSurfaces, futureSurfaces;
    for (uint32_t i = 0; i < executedFilters.GetSurfaceCount(true); ++i)
    {
        VP_SURFACE *inputSurface = GetCopyInstOfExtSurface(executedFilters.GetSurface(true, i));
        VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
        inputSurfaces.push_back(inputSurface);

        VP_SURFACE *pastSurface = GetCopyInstOfExtSurface(executedFilters.GetPastSurface(i));
        pastSurfaces.push_back(pastSurface ? pastSurface : nullptr);

        VP_SURFACE *futureSurface = GetCopyInstOfExtSurface(executedFilters.GetFutureSurface(i));
        futureSurfaces.push_back(futureSurface ? futureSurface : nullptr);
    }
    VP_SURFACE                  *outputSurface  = GetCopyInstOfExtSurface(executedFilters.GetSurface(false, 0));

    RESOURCE_ASSIGNMENT_HINT    resHint         = {};

    if (caps.bVebox && (caps.bDI || caps.bDiProcess2ndField))
    {
        m_isPastFrameVeboxDiUsed = true;
    }
    else
    {
        m_isPastFrameVeboxDiUsed = false;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(GetResourceHint(featurePool, executedFilters, resHint));

    if (nullptr == outputSurface && IsOutputSurfaceNeeded(caps))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AssignIntermediaSurface(caps, executedFilters));
        outputSurface  = GetCopyInstOfExtSurface(executedFilters.GetSurface(false, 0));
        VP_PUBLIC_CHK_NULL_RETURN(outputSurface);
    }

    VP_PUBLIC_CHK_STATUS_RETURN(AssignExecuteResource(caps, inputSurfaces, outputSurface,
        pastSurfaces, futureSurfaces, resHint, executedFilters.GetSurfacesSetting(), executedFilters));
    ++m_currentPipeIndex;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::GetUpdatedExecuteResource(std::vector<FeatureType> &featurePool, VP_EXECUTE_CAPS &caps, SwFilterPipe &swfilterPipe, VP_SURFACE_SETTING &surfSetting)
{
    VP_FUNC_CALL();

    std::vector<VP_SURFACE *> inputSurfaces, pastSurfaces, futureSurfaces;
    for (uint32_t i = 0; i < swfilterPipe.GetSurfaceCount(true); ++i)
    {
        VP_SURFACE *inputSurface = GetCopyInstOfExtSurface(swfilterPipe.GetSurface(true, i));
        VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
        inputSurfaces.push_back(inputSurface);

        VP_SURFACE *pastSurface = GetCopyInstOfExtSurface(swfilterPipe.GetPastSurface(i));
        pastSurfaces.push_back(pastSurface ? pastSurface : nullptr);

        VP_SURFACE *futureSurface = GetCopyInstOfExtSurface(swfilterPipe.GetFutureSurface(i));
        futureSurfaces.push_back(futureSurface ? futureSurface : nullptr);
    }
    VP_SURFACE *outputSurface  = GetCopyInstOfExtSurface(swfilterPipe.GetSurface(false, 0));

    RESOURCE_ASSIGNMENT_HINT resHint = {};

    VP_PUBLIC_CHK_STATUS_RETURN(GetResourceHint(featurePool, swfilterPipe, resHint));

    VP_PUBLIC_CHK_STATUS_RETURN(AssignExecuteResource(caps, inputSurfaces, outputSurface,
        pastSurfaces, futureSurfaces, resHint, surfSetting, swfilterPipe));
    ++m_currentPipeIndex;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignExecuteResource(VP_EXECUTE_CAPS& caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface,
    std::vector<VP_SURFACE *> &pastSurfaces, std::vector<VP_SURFACE *> &futureSurfaces, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting, SwFilterPipe& executedFilters)
{
    VP_FUNC_CALL();

    surfSetting.Clean();

    if (caps.bVebox || caps.bDnKernelUpdate)
    {
        // Create Vebox Resources
        VP_PUBLIC_CHK_STATUS_RETURN(AssignVeboxResource(caps, inputSurfaces[0], outputSurface, pastSurfaces[0], futureSurfaces[0], resHint, surfSetting, executedFilters));
    }

    if (caps.bRender)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AssignRenderResource(caps, inputSurfaces, outputSurface, pastSurfaces, futureSurfaces, resHint, surfSetting, executedFilters));
    }

    return MOS_STATUS_SUCCESS;
}

VPHAL_CSPACE GetDemosaicOutputColorSpace(VPHAL_CSPACE colorSpace)
{
    return IS_COLOR_SPACE_BT2020(colorSpace) ? CSpace_BT2020_RGB : CSpace_sRGB;
}

MOS_FORMAT GetDemosaicOutputFormat(VPHAL_CSPACE colorSpace)
{
    return IS_COLOR_SPACE_BT2020(colorSpace) ? Format_R10G10B10A2 : Format_A8B8G8R8;
}

MOS_STATUS GetVeboxOutputParams(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat, MOS_TILE_TYPE inputTileType, MOS_FORMAT outputFormat,
                                MOS_FORMAT &veboxOutputFormat, MOS_TILE_TYPE &veboxOutputTileType, VPHAL_CSPACE colorSpaceOutput)
{
    VP_FUNC_CALL();

    // Vebox Chroma Co-Sited downsampleing is part of VEO. It only affects format of vebox output surface, but not
    // affect sfc input format, that's why different logic between GetSfcInputFormat and GetVeboxOutputParams.
    // Check DI first and downsampling to NV12 if possible to save bandwidth no matter IECP enabled or not.
    if (executeCaps.b3DlutOutput)
    {
        if (IS_RGB64_FLOAT_FORMAT(outputFormat))  // SFC output FP16, YUV->ABGR16
        {
            veboxOutputFormat = Format_A16B16G16R16;
        }
        else
        {
            veboxOutputFormat = IS_COLOR_SPACE_BT2020(colorSpaceOutput) ? Format_R10G10B10A2 : Format_A8B8G8R8;
        }
        veboxOutputTileType = inputTileType;
    }
    else if (executeCaps.bDI || executeCaps.bDiProcess2ndField)
    {
        // NV12 will be used if target output is not YUV2 to save bandwidth.
        if (outputFormat == Format_YUY2)
        {
            veboxOutputFormat = Format_YUY2;
        }
        else
        {
            veboxOutputFormat = Format_NV12;
        }
        veboxOutputTileType = MOS_TILE_Y;
    }
    else if (executeCaps.bIECP && executeCaps.bCGC && executeCaps.bBt2020ToRGB)
    {
        veboxOutputFormat   = Format_A8B8G8R8;
        veboxOutputTileType = inputTileType;
    }
    else if (executeCaps.bIECP)
    {
        // Upsampling to yuv444 for IECP input/output.
        // To align with legacy path, need to check whether inputFormat can also be used for IECP case,
        // in which case IECP down sampling will be applied.
        veboxOutputFormat = Format_AYUV;
        veboxOutputTileType = inputTileType;
    }
    else if (executeCaps.bDemosaicInUse)
    {
        veboxOutputFormat = GetDemosaicOutputFormat(colorSpaceOutput);
        veboxOutputTileType = inputTileType;
    }
    else
    {
        veboxOutputFormat = inputFormat;
        veboxOutputTileType = inputTileType;
    }

    return MOS_STATUS_SUCCESS;
}


MOS_FORMAT GetSfcInputFormat(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat, VPHAL_CSPACE colorSpaceOutput, MOS_FORMAT outputFormat)
{
    VP_FUNC_CALL();

    // Vebox Chroma Co-Sited downsampling is part of VEO. It only affects format of vebox output surface, but not
    // affect sfc input format, that's why different logic between GetSfcInputFormat and GetVeboxOutputParams.
    // Check HDR case first, since 3DLUT output is fixed RGB32.
    // Then Check IECP, since IECP is done after DI, and the vebox downsampling not affect the vebox input.
    if (executeCaps.b3DlutOutput)
    {
        if (IS_RGB64_FLOAT_FORMAT(outputFormat))    // SFC output FP16, YUV->ABGR16
        {
            return Format_A16B16G16R16;
        }
        else
        {
            return IS_COLOR_SPACE_BT2020(colorSpaceOutput) ? Format_R10G10B10A2 : Format_A8B8G8R8;
        }
    }
    else if (executeCaps.bIECP && executeCaps.bCGC && executeCaps.bBt2020ToRGB)
    {
        // Upsampling to RGB444, and using ABGR as Vebox output
        return Format_A8B8G8R8;
    }
    else if (executeCaps.bIECP)
    {
        // Upsampling to yuv444 for IECP input/output.
        // To align with legacy path, need to check whether inputFormat can also be used for IECP case,
        // in which case IECP down sampling will be applied.
        return Format_AYUV;
    }
    else if (executeCaps.bDI)
    {
        // If the input is 4:2:0, then chroma data is doubled vertically to 4:2:2
        // For executeCaps.bDiProcess2ndField, no DI enabled in vebox, so no need
        // set to YUY2 here.
        return Format_YUY2;
    }
    else if (executeCaps.bDemosaicInUse)
    {
        return GetDemosaicOutputFormat(colorSpaceOutput);
    }

    return inputFormat;
}

MOS_STATUS VpResourceManager::ReAllocateVeboxOutputSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface,  bool &allocated)
{
    VP_FUNC_CALL();

    MOS_RESOURCE_MMC_MODE           surfCompressionMode = MOS_MMC_DISABLED;
    bool                            bSurfCompressible   = false;
    uint32_t                        i                   = 0;
    auto                           *skuTable            = m_osInterface.pfnGetSkuTable(&m_osInterface);
    Mos_MemPool                     memTypeSurfVideoMem = MOS_MEMPOOL_VIDEOMEMORY;

    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface->osSurface);

    if (skuTable && MEDIA_IS_SKU(skuTable, FtrLimitedLMemBar))
    {
        memTypeSurfVideoMem = MOS_MEMPOOL_DEVICEMEMORY;
    }

    MOS_FORMAT      veboxOutputFormat                   = inputSurface->osSurface->Format;
    MOS_TILE_TYPE   veboxOutputTileType                 = inputSurface->osSurface->TileType;

    VP_PUBLIC_CHK_STATUS_RETURN(GetVeboxOutputParams(caps, inputSurface->osSurface->Format, inputSurface->osSurface->TileType,
                                            outputSurface->osSurface->Format, veboxOutputFormat, veboxOutputTileType, outputSurface->ColorSpace));

    allocated = false;

    bool enableVeboxOutputSurf = false;
    if (m_vpUserFeatureControl)
    {
        enableVeboxOutputSurf = m_vpUserFeatureControl->IsVeboxOutputSurfEnabled();
    }

    bSurfCompressible   = inputSurface->osSurface->bCompressible;
    surfCompressionMode = inputSurface->osSurface->CompressionMode;

    if (m_currentFrameIds.pastFrameAvailable && m_currentFrameIds.futureFrameAvailable)
    {
        // Not switch back to 2 after being set to 4.
        m_veboxOutputCount = 4;
    }

    bool isVppInterResourceLocakable = enableVeboxOutputSurf ? VPP_INTER_RESOURCE_LOCKABLE : VPP_INTER_RESOURCE_NOTLOCKABLE;

    for (i = 0; i < m_veboxOutputCount; i++)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_veboxOutput[i],
            "VeboxSurfaceOutput",
            veboxOutputFormat,
            MOS_GFXRES_2D,
            veboxOutputTileType,
            inputSurface->osSurface->dwWidth,
            inputSurface->osSurface->dwHeight,
            bSurfCompressible,
            surfCompressionMode,
            allocated,
            false,
            IsDeferredResourceDestroyNeeded(),
            MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF,
            MOS_TILE_UNSET_GMM,
            memTypeSurfVideoMem,
            isVppInterResourceLocakable));

        m_veboxOutput[i]->ColorSpace = inputSurface->ColorSpace;
        m_veboxOutput[i]->rcDst      = inputSurface->rcDst;
        m_veboxOutput[i]->rcSrc      = inputSurface->rcSrc;
        m_veboxOutput[i]->rcMaxSrc   = inputSurface->rcMaxSrc;

        m_veboxOutput[i]->SampleType = SAMPLE_PROGRESSIVE;
    }

    if (allocated)
    {
        // Report Compress Status
        if (m_veboxOutput[0]->osSurface)
        {
            m_reporting.GetFeatures().ffdiCompressible = m_veboxOutput[0]->osSurface->bIsCompressed;
            m_reporting.GetFeatures().ffdiCompressMode = (uint8_t)m_veboxOutput[0]->osSurface->CompressionMode;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::ReAllocateVeboxDenoiseOutputSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, bool &allocated)
{
    VP_FUNC_CALL();

    MOS_RESOURCE_MMC_MODE           surfCompressionMode = MOS_MMC_DISABLED;
    bool                            bSurfCompressible   = false;
    MOS_TILE_MODE_GMM               tileModeByForce     = MOS_TILE_UNSET_GMM;
    auto *                          skuTable            = m_osInterface.pfnGetSkuTable(&m_osInterface);
    auto *                          waTable             = m_osInterface.pfnGetWaTable(&m_osInterface);
    Mos_MemPool                     memTypeSurfVideoMem = MOS_MEMPOOL_VIDEOMEMORY;
    uint32_t                        dwHeight;
    MOS_TILE_TYPE                   TileType;

    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);

    if (skuTable)
    {
        //DN output surface must be tile64 only when input format is bayer
        if (!MEDIA_IS_SKU(skuTable, FtrTileY) &&
            IS_BAYER_FORMAT(inputSurface->osSurface->Format))
        {
            tileModeByForce = MOS_TILE_64_GMM;
        }

        if (MEDIA_IS_SKU(skuTable, FtrLimitedLMemBar))
        {
            memTypeSurfVideoMem = MOS_MEMPOOL_DEVICEMEMORY;
        }
    }

    allocated = false;

    bSurfCompressible   = inputSurface->osSurface->bCompressible;
    surfCompressionMode = inputSurface->osSurface->CompressionMode;

    if (caps.bCappipe)
    {
        bSurfCompressible   = false;
        surfCompressionMode = MOS_MMC_DISABLED;
        // Add 4 to input height for DN and STMM if Bayer Pattern offset is 10 or 11
        if (IS_BAYER_GRBG_FORMAT(inputSurface->osSurface->Format) ||
            IS_BAYER_GBRG_FORMAT(inputSurface->osSurface->Format))
        {
            dwHeight = inputSurface->osSurface->dwHeight + 4;
        }
        else
        {
            dwHeight = inputSurface->osSurface->dwHeight;
        }
        // For Bayer pattern inputs only the Current Denoised Output/Previous Denoised Input are in Tile-Y
        TileType = IS_BAYER_FORMAT(inputSurface->osSurface->Format) ? MOS_TILE_Y : inputSurface->osSurface->TileType;
    }
    else
    {
        dwHeight = inputSurface->osSurface->dwHeight;
        TileType = inputSurface->osSurface->TileType;
    }

    for (uint32_t i = 0; i < VP_NUM_DN_SURFACES; i++)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_veboxDenoiseOutput[i],
            "VeboxFFDNSurface",
            inputSurface->osSurface->Format,
            MOS_GFXRES_2D,
            TileType,
            inputSurface->osSurface->dwWidth,
            dwHeight,
            bSurfCompressible,
            surfCompressionMode,
            allocated,
            false,
            IsDeferredResourceDestroyNeeded(),
            MOS_HW_RESOURCE_USAGE_VP_INPUT_REFERENCE_FF,
            tileModeByForce,
            memTypeSurfVideoMem,
            VPP_INTER_RESOURCE_NOTLOCKABLE));

        // DN output surface state should be able to share with vebox input surface state
        if (m_veboxDenoiseOutput[i]->osSurface &&
            (m_veboxDenoiseOutput[i]->osSurface->YoffsetForUplane != inputSurface->osSurface->YoffsetForUplane || m_veboxDenoiseOutput[i]->osSurface->YoffsetForVplane != inputSurface->osSurface->YoffsetForVplane))
        {
            VP_PUBLIC_ASSERTMESSAGE("Vebox surface state of DN output surface doesn't align with input surface!");
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNKNOWN);
        }

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
            if (m_veboxDenoiseOutput[i]->osSurface)
            {
                m_reporting.GetFeatures().ffdnCompressible = m_veboxDenoiseOutput[i]->osSurface->bIsCompressed;
                m_reporting.GetFeatures().ffdnCompressMode = (uint8_t)m_veboxDenoiseOutput[i]->osSurface->CompressionMode;
            }
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
    VP_FUNC_CALL();

    MOS_RESOURCE_MMC_MODE           surfCompressionMode = MOS_MMC_DISABLED;
    bool                            bSurfCompressible   = false;
    uint32_t                        i                   = 0;
    MOS_TILE_MODE_GMM               tileModeByForce     = MOS_TILE_UNSET_GMM;
    auto *                          skuTable            = m_osInterface.pfnGetSkuTable(&m_osInterface);
    Mos_MemPool                     memTypeHistStat     = GetHistStatMemType(caps);
    uint32_t                        dwHeight;

    //STMM surface can be not lockable, if secure mode is enabled
    bool isSTMMNotLockable = caps.bSecureVebox;

    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);

    if (skuTable && !MEDIA_IS_SKU(skuTable, FtrTileY))
    {
        tileModeByForce = MOS_TILE_64_GMM;
    }

    if (caps.bCappipe)
    {
        // Add 4 to input height for DN and STMM if Bayer Pattern offset is 10 or 11
        if (IS_BAYER_GRBG_FORMAT(inputSurface->osSurface->Format) ||
            IS_BAYER_GBRG_FORMAT(inputSurface->osSurface->Format))
        {
            dwHeight = inputSurface->osSurface->dwHeight + 4;
        }
        else
        {
            dwHeight = inputSurface->osSurface->dwHeight;
        }
    }
    else
    {
        dwHeight = inputSurface->osSurface->dwHeight;
    }

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
            dwHeight,
            bSurfCompressible,
            surfCompressionMode,
            allocated,
            false,
            IsDeferredResourceDestroyNeeded(),
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF,
            tileModeByForce,
            memTypeHistStat,
            isSTMMNotLockable));

        if (allocated)
        {
            VP_PUBLIC_CHK_NULL_RETURN(m_veboxSTMMSurface[i]);
            // Report Compress Status
            m_reporting.GetFeatures().stmmCompressible = bSurfCompressible;
            m_reporting.GetFeatures().stmmCompressMode = (uint8_t)surfCompressionMode;
        }
    }
    return MOS_STATUS_SUCCESS;
}

void VpResourceManager::DestoryVeboxOutputSurface()
{
    VP_FUNC_CALL();

    for (uint32_t i = 0; i < VP_MAX_NUM_VEBOX_SURFACES; i++)
    {
        m_allocator.DestroyVpSurface(m_veboxOutput[i], IsDeferredResourceDestroyNeeded());
    }
}

void VpResourceManager::DestoryVeboxDenoiseOutputSurface()
{
    VP_FUNC_CALL();

    for (uint32_t i = 0; i < VP_NUM_DN_SURFACES; i++)
    {
        m_allocator.DestroyVpSurface(m_veboxDenoiseOutput[i], IsDeferredResourceDestroyNeeded());
    }
}

void VpResourceManager::DestoryVeboxSTMMSurface()
{
    VP_FUNC_CALL();

    // Free DI history buffers (STMM = Spatial-temporal motion measure)
    for (uint32_t i = 0; i < VP_NUM_STMM_SURFACES; i++)
    {
        m_allocator.DestroyVpSurface(m_veboxSTMMSurface[i], IsDeferredResourceDestroyNeeded());
    }
}

MOS_STATUS VpResourceManager::FillLinearBufferWithEncZero(VP_SURFACE *surface, uint32_t width, uint32_t height)
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

uint32_t VpResourceManager::Get3DLutSize(bool is33LutSizeEnabled, uint32_t &lutWidth, uint32_t &lutHeight)
{
    VP_FUNC_CALL();

    if (is33LutSizeEnabled)
    {
        lutWidth  = LUT33_SEG_SIZE * 2;
        lutHeight = LUT33_SEG_SIZE * LUT33_MUL_SIZE;
        VP_RENDER_NORMALMESSAGE("3DLut table is used 33 lutsize.");
        return VP_VEBOX_HDR_3DLUT33;
    }
    else
    {
        lutWidth = LUT65_SEG_SIZE * 2;
        lutHeight = LUT65_SEG_SIZE * LUT65_MUL_SIZE;
        return VP_VEBOX_HDR_3DLUT65;
    }
}

uint32_t VpResourceManager::Get1DLutSize()
{
    VP_FUNC_CALL();

    return SHAPE_1K_LOOKUP_SIZE;
}

Mos_MemPool VpResourceManager::GetHistStatMemType(VP_EXECUTE_CAPS &caps)
{
    VP_FUNC_CALL();

    return MOS_MEMPOOL_VIDEOMEMORY;
}

MOS_STATUS VpResourceManager::AllocateVeboxResource(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface)
{
    VP_FUNC_CALL();
    MOS_FORMAT                      format;
    MOS_TILE_TYPE                   TileType;
    uint32_t                        dwWidth;
    uint32_t                        dwHeight;
    uint32_t                        dwSize;
    uint32_t                        i;
    MOS_RESOURCE_MMC_MODE           surfCompressionMode        = MOS_MMC_DISABLED;
    bool                            bSurfCompressible          = false;
    bool                            bAllocated                 = false;
    uint8_t                         InitValue                  = 0;
    Mos_MemPool                     memTypeHistStat            = GetHistStatMemType(caps);
    bool                            isStatisticsBufNotLockable = false;

    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurface->osSurface);

    // change the init value when null hw is enabled
    if (m_osInterface.bNullHwIsEnabled)
    {
        InitValue = 0x80;
    }

    if (IS_VP_VEBOX_DN_ONLY(caps))
    {
        bSurfCompressible   = inputSurface->osSurface->bCompressible;
        surfCompressionMode = inputSurface->osSurface->CompressionMode;
    }
    else
    {
        bSurfCompressible   = true;
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
            m_currentDnOutput   = 0;
            m_pastDnOutputValid = false;
        }
    }
    else
    {
        DestoryVeboxDenoiseOutputSurface();
        m_pastDnOutputValid = false;
    }

    if (VeboxSTMMNeeded(caps, false))
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
    if (caps.bDnKernelUpdate)
    {
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
            IsDeferredResourceDestroyNeeded(),
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
            IsDeferredResourceDestroyNeeded(),
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF));

        if (bAllocated)
        {
            // initialize Spatial Attributes Configuration Surface
            VP_PUBLIC_CHK_STATUS_RETURN(InitVeboxSpatialAttributesConfiguration());
        }
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
        IsDeferredResourceDestroyNeeded(),
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_WRITE_FF));

    m_isHistogramReallocated = bAllocated;

    if (bAllocated && m_osInterface.bNullHwIsEnabled)
    {
        // Initialize veboxRgbHistogram Surface
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.OsFillResource(
            &(m_veboxRgbHistogram->osSurface->OsResource),
            dwSize,
            InitValue));
    }

    // Allocate Statistics State Surface----------------------------------------
    // Width to be a aligned on 64 bytes and height is 1/4 the height
    // Per frame information written twice per frame for 2 slices
    // Surface to be a rectangle aligned with dwWidth to get proper dwSize
    // APG PAth need to make sure input surface width/height is what to processed width/Height
    uint32_t statistic_size = m_vpPlatformInterface.VeboxQueryStaticSurfaceSize();
    dwWidth                 = MOS_ALIGN_CEIL(inputSurface->osSurface->dwWidth, 64);
    dwHeight                = MOS_ROUNDUP_DIVIDE(inputSurface->osSurface->dwHeight, 4) +
                MOS_ROUNDUP_DIVIDE(statistic_size * sizeof(uint32_t), dwWidth);

    if (caps.b1stPassOfSfc2PassScaling)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(ReAllocateVeboxStatisticsSurface(m_veboxStatisticsSurfacefor1stPassofSfc2Pass, caps, inputSurface, dwWidth, dwHeight));
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(ReAllocateVeboxStatisticsSurface(m_veboxStatisticsSurface, caps, inputSurface, dwWidth, dwHeight));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(Allocate3DLut(caps));

    if (caps.b1K1DLutInUse)
    {
        dwSize = Get1DLutSize();
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_vebox1DLookUpTables,
            "Dv1K1DLutTableSurface",
            Format_Buffer,
            MOS_GFXRES_BUFFER,
            MOS_TILE_LINEAR,
            dwSize,
            1,
            false,
            MOS_MMC_DISABLED,
            bAllocated,
            false,
            IsDeferredResourceDestroyNeeded()));
        if (!bAllocated && !caps.bDV)
        {
            caps.b1K1DLutInited = 1;
        }
    }
    // cappipe
    if (caps.enableSFCLinearOutputByTileConvert)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_innerTileConvertInput,
            "TempTargetSurface",
            outputSurface->osSurface->Format,
            MOS_GFXRES_2D,
            MOS_TILE_Y,
            outputSurface->osSurface->dwWidth,
            outputSurface->osSurface->dwHeight,
            false,
            MOS_MMC_DISABLED,
            bAllocated,
            false,
            IsDeferredResourceDestroyNeeded()));

        m_innerTileConvertInput->ColorSpace = outputSurface->ColorSpace;
        m_innerTileConvertInput->rcSrc      = outputSurface->rcSrc;
        m_innerTileConvertInput->rcDst      = outputSurface->rcDst;
        m_innerTileConvertInput->rcMaxSrc   = outputSurface->rcMaxSrc;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::Allocate3DLut(VP_EXECUTE_CAPS& caps)
{
    VP_FUNC_CALL();
    uint32_t                        size = 0;
    bool                            isAllocated          = false;

    if (caps.bHDR3DLUT || caps.b3DLutCalc)
    {
        // HDR
        uint32_t lutWidth = 0;
        uint32_t lutHeight = 0;
        size = Get3DLutSize(caps.bHdr33lutsize, lutWidth, lutHeight);
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
            m_vebox3DLookUpTables,
            "Vebox3DLutTableSurface",
            Format_Buffer,
            MOS_GFXRES_BUFFER,
            MOS_TILE_LINEAR,
            size,
            1,
            false,
            MOS_MMC_DISABLED,
            isAllocated,
            false,
            IsDeferredResourceDestroyNeeded(),
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER));

    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AllocateResourceFor3DLutKernel(VP_EXECUTE_CAPS& caps)
{
    VP_FUNC_CALL();
    uint32_t    size = 0;
    bool        isAllocated = false;
    uint32_t    lutWidth = 0;
    uint32_t    lutHeight = 0;

    uint32_t sizeOf3DLut = Get3DLutSize(caps.bHdr33lutsize, lutWidth, lutHeight);

    if (caps.bHdr33lutsize)
    {
        if (VP_VEBOX_HDR_3DLUT33 != sizeOf3DLut)
        {
            VP_PUBLIC_ASSERTMESSAGE("3DLutSize(%x) != VP_VEBOX_HDR_3DLUT33(%x)", sizeOf3DLut, VP_VEBOX_HDR_3DLUT33);
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }
    else
    {
        if (VP_VEBOX_HDR_3DLUT65 != sizeOf3DLut)
        {
            VP_PUBLIC_ASSERTMESSAGE("3DLutSize(%x) != VP_VEBOX_HDR_3DLUT65(%x)", sizeOf3DLut, VP_VEBOX_HDR_3DLUT65);
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(Allocate3DLut(caps));

    uint32_t size_coef     = 8 * 8 * 4;

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_3DLutKernelCoefSurface,
        "3DLutKernelCoefSurface",
        Format_Buffer,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        size_coef,
        1,
        false,
        MOS_MMC_DISABLED,
        isAllocated,
        false,
        IsDeferredResourceDestroyNeeded(),
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::Assign3DLutKernelResource(VP_EXECUTE_CAPS &caps, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(AllocateResourceFor3DLutKernel(caps));

    surfSetting.surfGroup.insert(std::make_pair(SurfaceType3DLut, m_vebox3DLookUpTables));
    surfSetting.surfGroup.insert(std::make_pair(SurfaceType3DLutCoef, m_3DLutKernelCoefSurface));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AllocateResourceForHVSKernel(VP_EXECUTE_CAPS &caps)
{
    VP_FUNC_CALL();
    bool     isAllocated = false;

    uint32_t size  = 40 * 4;

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        m_veboxDnHVSTables,
        "HVSKernelTableSurface",
        Format_Buffer,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        size,
        1,
        false,
        MOS_MMC_DISABLED,
        isAllocated,
        false,
        IsDeferredResourceDestroyNeeded(),
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignHVSKernelResource(VP_EXECUTE_CAPS &caps, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(AllocateResourceForHVSKernel(caps));

    surfSetting.surfGroup.insert(std::make_pair(SurfaceTypeHVSTable, m_veboxDnHVSTables));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignSurface(VP_EXECUTE_CAPS caps, VEBOX_SURFACE_ID &surfaceId, SurfaceType surfaceType, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface, VP_SURFACE *pastSurface, VP_SURFACE *futureSurface, VP_SURFACE_GROUP &surfGroup)
{
    VP_FUNC_CALL();

    switch (surfaceId)
    {
    case VEBOX_SURFACE_INPUT:
        if (nullptr == inputSurface)
        {
            VP_PUBLIC_ASSERTMESSAGE("inputSurface should not be nullptr when surfaceId being VEBOX_SURFACE_INPUT.");
            break;
        }
        surfGroup.insert(std::make_pair(surfaceType, inputSurface));
        break;
    case VEBOX_SURFACE_OUTPUT:
        if (nullptr == outputSurface)
        {
            VP_PUBLIC_ASSERTMESSAGE("outputSurface should not be nullptr when surfaceId being VEBOX_SURFACE_OUTPUT.");
            break;
        }
        surfGroup.insert(std::make_pair(surfaceType, outputSurface));
        break;
    case VEBOX_SURFACE_PAST_REF:
        if (caps.bDN && m_pastDnOutputValid)
        {
            surfGroup.insert(std::make_pair(surfaceType, m_veboxDenoiseOutput[(m_currentDnOutput + 1) & 1]));
        }
        else
        {
            auto curDnOutputSurface = m_veboxDenoiseOutput[m_currentDnOutput];

            if (nullptr == pastSurface)
            {
                VP_PUBLIC_ASSERTMESSAGE("pastSurface should not be nullptr when surfaceId being VEBOX_SURFACE_PAST_REF.");
                break;
            }

            if (!caps.bDN                               ||
                nullptr == curDnOutputSurface           ||
                // When FtrTileY is false, DN output surface will be tile64 when input is bayer format,
                // while pastSurface passed by OS maybe tile4, which is different from DN output surface.
                // For such case, passSurface cannot be used, as vebox previous input surface and vebox
                // DN output surface must share same setting. The derive pitch in vebox output surface
                // state is for both of them. Check pitch to handle it.
                pastSurface->osSurface->dwPitch == curDnOutputSurface->osSurface->dwPitch)
            {
                surfGroup.insert(std::make_pair(surfaceType, pastSurface));
            }
            else
            {
                // DN case with m_pastDnOutputValid being false. pastSurface cannot be used here as pitch
                // of pastSurface is different from current DN output surface.
                VP_PUBLIC_NORMALMESSAGE("Do not use pastSurface. pastSurf (TileModeGmm: %d, pitch: %d) vs curDnOutputSurface (TileModeGmm: %d, pitch: %d)",
                    pastSurface->osSurface->TileModeGMM,
                    pastSurface->osSurface->dwPitch,
                    curDnOutputSurface->osSurface->TileModeGMM,
                    curDnOutputSurface->osSurface->dwPitch);
            }
        }
        break;
    case VEBOX_SURFACE_FUTURE_REF:
        if (nullptr == futureSurface)
        {
            VP_PUBLIC_ASSERTMESSAGE("futureSurface should not be nullptr when surfaceId being VEBOX_SURFACE_FUTURE_REF.");
            break;
        }
        surfGroup.insert(std::make_pair(surfaceType, futureSurface));
        break;
    case VEBOX_SURFACE_FRAME0:
        surfGroup.insert(std::make_pair(surfaceType, m_veboxOutput[(m_currentDnOutput + 0) % m_veboxOutputCount]));
        break;
    case VEBOX_SURFACE_FRAME1:
        surfGroup.insert(std::make_pair(surfaceType, m_veboxOutput[(m_currentDnOutput + 1) % m_veboxOutputCount]));
        break;
    case VEBOX_SURFACE_FRAME2:
        surfGroup.insert(std::make_pair(surfaceType, m_veboxOutput[(m_currentDnOutput + 2) % m_veboxOutputCount]));
        break;
    case VEBOX_SURFACE_FRAME3:
        surfGroup.insert(std::make_pair(surfaceType, m_veboxOutput[(m_currentDnOutput + 3) % m_veboxOutputCount]));
        break;
    default:
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignVeboxResource(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface,
    VP_SURFACE *pastSurface, VP_SURFACE *futureSurface, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting, SwFilterPipe& executedFilters)
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
    auto&                           surfGroup = surfSetting.surfGroup;

    // Render case reuse vebox resource, and don`t need re-allocate.
    if (!caps.bRender ||
        (caps.bRender && caps.bDnKernelUpdate))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AllocateVeboxResource(caps, inputSurface, outputSurface));
    }

    if (caps.bDI || caps.bDiProcess2ndField)
    {
        bool b60fpsDi = resHint.b60fpsDi || caps.bDiProcess2ndField;
        VEBOX_SURFACES_CONFIG cfg(b60fpsDi, caps.bSFC, m_sameSamples, m_outOfBound, m_currentFrameIds.pastFrameAvailable,
            m_currentFrameIds.futureFrameAvailable, IsInterleaveFirstField(inputSurface->SampleType));
        auto it = m_veboxSurfaceConfigMap.find(cfg.value);
        if (m_veboxSurfaceConfigMap.end() == it)
        {
            VP_PUBLIC_ASSERTMESSAGE("SurfaceConfig is invalid, cfg.value = %d", cfg.value);
            VP_PUBLIC_ASSERTMESSAGE("b64Di = %d, sfcEnable = %d, sameSample = %d, outOfBound = %d, pastframeAvailable = %d, future frameAvaliable = %d, FirstDiFiels = %d",
                cfg.b64DI,
                cfg.sfcEnable,
                cfg.sameSample,
                cfg.outOfBound,
                cfg.pastFrameAvailable,
                cfg.futureFrameAvailable,
                cfg.firstDiField);
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        auto surfaces = it->second;
        VP_PUBLIC_CHK_STATUS_RETURN(AssignSurface(caps, surfaces.currentInputSurface, SurfaceTypeVeboxInput, inputSurface, outputSurface, pastSurface, futureSurface, surfSetting.surfGroup));
        VP_PUBLIC_CHK_STATUS_RETURN(AssignSurface(caps, surfaces.pastInputSurface, SurfaceTypeVeboxPreviousInput, inputSurface, outputSurface, pastSurface, futureSurface, surfSetting.surfGroup));
        VP_PUBLIC_CHK_STATUS_RETURN(AssignSurface(caps, surfaces.currentOutputSurface, SurfaceTypeVeboxCurrentOutput, inputSurface, outputSurface, pastSurface, futureSurface, surfSetting.surfGroup));
        VP_PUBLIC_CHK_STATUS_RETURN(AssignSurface(caps, surfaces.pastOutputSurface, SurfaceTypeVeboxPreviousOutput, inputSurface, outputSurface, pastSurface, futureSurface, surfSetting.surfGroup));

        if (caps.bDN)
        {
            // Insert DN output surface
            surfGroup.insert(std::make_pair(SurfaceTypeDNOutput, m_veboxDenoiseOutput[m_currentDnOutput]));
        }

        caps.bRefValid = surfGroup.find(SurfaceTypeVeboxPreviousInput) != surfGroup.end();
    }
    else
    {
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
    }

    if (VeboxSTMMNeeded(caps, true))
    {
        // Insert STMM input surface
        surfGroup.insert(std::make_pair(SurfaceTypeSTMMIn, m_veboxSTMMSurface[m_currentStmmIndex]));
        // Insert STMM output surface
        surfGroup.insert(std::make_pair(SurfaceTypeSTMMOut, m_veboxSTMMSurface[(m_currentStmmIndex + 1) & 1]));
    }

#if VEBOX_AUTO_DENOISE_SUPPORTED
    if (caps.bDnKernelUpdate)
    {
        // Insert Vebox auto DN noise level surface
        surfGroup.insert(std::make_pair(SurfaceTypeAutoDNNoiseLevel, m_veboxDNTempSurface));
        // Insert Vebox auto DN spatial config surface/buffer
        surfGroup.insert(std::make_pair(SurfaceTypeAutoDNSpatialConfig, m_veboxDNSpatialConfigSurface));
    }
#endif

    // Insert Vebox histogram surface
    surfGroup.insert(std::make_pair(SurfaceTypeLaceAceRGBHistogram, m_veboxRgbHistogram));

    // Insert Vebox statistics surface
    if (caps.b1stPassOfSfc2PassScaling)
    {
        surfGroup.insert(std::make_pair(SurfaceTypeStatistics, m_veboxStatisticsSurfacefor1stPassofSfc2Pass));
    }
    else
    {
        surfGroup.insert(std::make_pair(SurfaceTypeStatistics, m_veboxStatisticsSurface));
    }
    surfSetting.dwVeboxPerBlockStatisticsHeight = m_dwVeboxPerBlockStatisticsHeight;
    surfSetting.dwVeboxPerBlockStatisticsWidth  = m_dwVeboxPerBlockStatisticsWidth;

    if (VeboxHdr3DlutNeeded(caps))
    {
        // Insert Vebox 3Dlut surface
        surfGroup.insert(std::make_pair(SurfaceType3DLut, m_vebox3DLookUpTables));
    }

    if (resHint.isHVSTableNeeded)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_veboxDnHVSTables);
        // Insert Vebox HVS DN surface
        surfGroup.insert(std::make_pair(SurfaceTypeHVSTable, m_veboxDnHVSTables));
    }

    if (caps.b1K1DLutInUse)
    {
        // Insert DV 1Dlut surface
        surfGroup.insert(std::make_pair(SurfaceType1k1dLut, m_vebox1DLookUpTables));
    }

    if (caps.enableSFCLinearOutputByTileConvert)
    {
        surfGroup.insert(std::make_pair(SurfaceTypeInnerTileConvertInput, m_innerTileConvertInput));
    }

    // Update previous Dn output flag for next frame to use.
    if (surfGroup.find(SurfaceTypeDNOutput) != surfGroup.end() || m_sameSamples && m_pastDnOutputValid)
    {
        m_pastDnOutputValid = true;
    }
    else
    {
        m_pastDnOutputValid = false;
    }

    return MOS_STATUS_SUCCESS;
}

bool VpResourceManager::IsOutputSurfaceNeeded(VP_EXECUTE_CAPS caps)
{
    VP_FUNC_CALL();

    // check whether intermedia surface needed to create based on caps
    if (caps.bDnKernelUpdate ||  // State Heap as putput, but it was not tracked in resource manager yet
        caps.bVeboxSecureCopy)   // State Heap as putput, but it was not tracked in resource manager yet
    {
        return false;
    }
    else
    {
        return true;
    }
}

VP_SURFACE* VpResourceManager::GetVeboxOutputSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *outputSurface)
{
    VP_FUNC_CALL();

    bool enableVeboxOutputSurf = false;
    if (m_vpUserFeatureControl)
    {
        enableVeboxOutputSurf = m_vpUserFeatureControl->IsVeboxOutputSurfEnabled();
    }

    if (caps.bRender)
    {
        // Place Holder when enable DI
        return nullptr;
    }

    if (!caps.bSFC) // Vebox output directlly to output surface
    {
        // RenderTarget will be assigned in VpVeboxCmdPacket::GetSurface.
        return outputSurface;
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
    else if (enableVeboxOutputSurf || caps.bDN)  // SFC + DN case
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
    VP_FUNC_CALL();
    bool enableVeboxOutputSurf = false;
    if (m_vpUserFeatureControl)
    {
        enableVeboxOutputSurf = m_vpUserFeatureControl->IsVeboxOutputSurfEnabled();
    }

    // If DN and/or Hotpixel are the only functions enabled then the only output is the Denoised Output
    // and no need vebox output.
    // For any other vebox features being enabled, vebox output surface is needed.
    if (enableVeboxOutputSurf   ||
        caps.bDI                ||
        caps.bQueryVariance     ||
        caps.bDiProcess2ndField ||
        caps.bIECP              ||
        caps.bCappipe           || 
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
    VP_FUNC_CALL();

    return caps.bDN;
}

bool VpResourceManager::VeboxHdr3DlutNeeded(VP_EXECUTE_CAPS &caps)
{
    VP_FUNC_CALL();

    return caps.bHDR3DLUT;
}

bool VpResourceManager::Vebox1DlutNeeded(VP_EXECUTE_CAPS &caps)
{
    VP_FUNC_CALL();

    return caps.bDV;
}

// In some case, STMM should not be destroyed even when not being used by current workload to maintain data,
// e.g. DI second field case.
// If queryAssignment == true, query whether STMM needed by current workload.
// If queryAssignment == false, query whether STMM needed to be allocated.
bool VpResourceManager::VeboxSTMMNeeded(VP_EXECUTE_CAPS& caps, bool queryAssignment)
{
    VP_FUNC_CALL();

    if (queryAssignment)
    {
        return caps.bDI || caps.bDN;
    }
    else
    {
        return caps.bDI || caps.bDiProcess2ndField || caps.bDN;
    }
}

MOS_STATUS VpResourceManager::ReAllocateVeboxStatisticsSurface(VP_SURFACE *&statisticsSurface, VP_EXECUTE_CAPS &caps, VP_SURFACE *inputSurface, uint32_t dwWidth, uint32_t dwHeight)
{
    VP_FUNC_CALL();
    
    bool        bAllocated                  = false;
    Mos_MemPool memTypeHistStat             = GetHistStatMemType(caps);
    //Statistics surface can be not lockable, if secure mode is enabled
    bool        isStatisticsBufNotLockable  = caps.bSecureVebox;
    uint8_t     InitValue                   = 0;

    // change the init value when null hw is enabled
    if (m_osInterface.bNullHwIsEnabled)
    {
        InitValue = 0x80;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator.ReAllocateSurface(
        statisticsSurface,
        "m_veboxStatisticsSurface",
        Format_Buffer,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        dwWidth,
        dwHeight,
        false,
        MOS_MMC_DISABLED,
        bAllocated,
        false,
        IsDeferredResourceDestroyNeeded(),
        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_WRITE_FF,
        MOS_TILE_UNSET_GMM,
        memTypeHistStat,
        isStatisticsBufNotLockable));

    if (bAllocated)
    {
        if (caps.bSecureVebox)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(FillLinearBufferWithEncZero(statisticsSurface, dwWidth, dwHeight));
        }
    }

    m_dwVeboxPerBlockStatisticsWidth  = dwWidth;
    m_dwVeboxPerBlockStatisticsHeight = MOS_ROUNDUP_DIVIDE(inputSurface->osSurface->dwHeight, 4);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpResourceManager::AssignHdrResource(VP_EXECUTE_CAPS &caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting, SwFilterPipe &executedFilters)
{
    VP_FUNC_CALL();

    if (m_hdrResourceManager == nullptr)
    {
        m_hdrResourceManager = MOS_New(VphdrResourceManager, m_allocator);
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_hdrResourceManager->AssignRenderResource(caps, inputSurfaces, outputSurface, resHint, surfSetting, executedFilters, m_osInterface, m_reporting, IsDeferredResourceDestroyNeeded()));

    return MOS_STATUS_SUCCESS;
}
};
