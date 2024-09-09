/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     vp_feature_report.cpp
//! \brief    vp feature report
//! \details  vp feature report class inlcuding:
//!           features, functions
//!
#include "vp_feature_report.h"
#include "vp_utils.h"
#include "vp_pipeline_adapter_base.h"

void VpFeatureReport::InitReportValue()
{
    VP_FUNC_CALL();

    m_features.iecp                = false;
    m_features.ief                 = false;
    m_features.denoise             = false;
    m_features.chromaDenoise       = false;
    m_features.deinterlaceMode     = VPHAL_DI_REPORT_PROGRESSIVE;
    m_features.scalingMode         = VPHAL_SCALING_NEAREST;
    m_features.outputPipeMode      = VPHAL_OUTPUT_PIPE_MODE_COMP;
    m_features.vpMMCInUse          = false;
    m_features.rtCompressible      = false;
    m_features.rtCompressMode      = 0;
    m_features.rtCacheSetting      = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
    m_features.rtOldCacheSetting   = 0;
#endif
    m_features.ffdiCompressible    = false;
    m_features.ffdiCompressMode    = 0;
    m_features.ffdnCompressible    = false;
    m_features.ffdnCompressMode    = 0;
    m_features.stmmCompressible    = false;
    m_features.stmmCompressMode    = 0;
    m_features.scalerCompressible  = false;
    m_features.scalerCompressMode  = 0;
    m_features.primaryCompressible = false;
    m_features.primaryCompressMode = 0;
    m_features.compositionMode     = VPHAL_NO_COMPOSITION;
    m_features.diScdMode           = false;
    m_features.veFeatureInUse      = false;
    m_features.hdrMode             = VPHAL_HDR_MODE_NONE;

    return;
}

void VpFeatureReport::SetConfigValues(
    PVP_CONFIG configValues,
    bool       traceEvent)
{
    VP_FUNC_CALL();

    // Report DI mode
    switch (m_features.deinterlaceMode)
    {
        case VPHAL_DI_REPORT_BOB:
        case VPHAL_DI_REPORT_ADI_BOB:
            configValues->dwCurrentDeinterlaceMode = VPDDI_BOB;
            break;
        case VPHAL_DI_REPORT_ADI:
        case VPHAL_DI_REPORT_FMD:
            configValues->dwCurrentDeinterlaceMode = VPDDI_ADI;
            break;
        case VPHAL_DI_REPORT_PROGRESSIVE:
        default:
            configValues->dwCurrentDeinterlaceMode = VPDDI_PROGRESSIVE;
            break;
    }

    // Report Scaling mode
    configValues->dwCurrentScalingMode = (m_features.scalingMode == VPHAL_SCALING_AVS) ? VPDDI_ADVANCEDSCALING : (m_features.scalingMode > VPHAL_SCALING_AVS) ? VPDDI_SUPERRESOLUTIONSCALING : VPDDI_SCALING;

    // Report HDR Mode
    configValues->dwCurrentHdrMode = m_features.hdrMode;

    // Report Output Pipe
    configValues->dwCurrentOutputPipeMode = m_features.outputPipeMode;

    // Report VE Feature In Use
    configValues->dwCurrentVEFeatureInUse = m_features.veFeatureInUse;

    // Report vp packet reused flag.
    configValues->isPacketReused = m_features.packetReused;

    // Report vp Dn enabled flag.
    configValues->isDnEnabled = m_features.denoise;

    // Report MMC status
    configValues->dwVPMMCInUse          = m_features.vpMMCInUse;
    configValues->dwRTCompressible      = m_features.rtCompressible;
    configValues->dwRTCompressMode      = m_features.rtCompressMode;
    configValues->dwFFDICompressible    = m_features.ffdiCompressible;
    configValues->dwFFDICompressMode    = m_features.ffdiCompressMode;
    configValues->dwFFDNCompressible    = m_features.ffdnCompressible;
    configValues->dwFFDNCompressMode    = m_features.ffdnCompressMode;
    configValues->dwSTMMCompressible    = m_features.stmmCompressible;
    configValues->dwSTMMCompressMode    = m_features.stmmCompressMode;
    configValues->dwScalerCompressible  = m_features.scalerCompressible;
    configValues->dwScalerCompressMode  = m_features.scalerCompressMode;
    configValues->dwPrimaryCompressible = m_features.primaryCompressible;
    configValues->dwPrimaryCompressMode = m_features.primaryCompressMode;

    // Report Render Target cache usage
    configValues->dwRTCacheSetting     = m_features.rtCacheSetting;
#if (_DEBUG || _RELEASE_INTERNAL)
    configValues->dwRTOldCacheSetting = m_features.rtOldCacheSetting;
#endif

    // Report In Place Compositon status
    configValues->dwCurrentCompositionMode = m_features.compositionMode;
    configValues->dwCurrentScdMode         = m_features.diScdMode;

    // Report Vebox Scalability
    configValues->dwCurrentVeboxScalability = m_features.VeboxScalability;

    configValues->dwCurrentSFCLinearOutputByTileConvert = m_features.sfcLinearOutputByTileConvert;

    // Report VP Apogeios
    configValues->dwCurrentVPApogeios       = m_features.VPApogeios;

    VP_PUBLIC_NORMALMESSAGE(
        "VP Feature Report: \
        OutputPipeMode %d, \
        HDRMode %d, \
        VEFeatureInUse %d, \
        ScalingMode %d, \
        DeinterlaceMode %d, \
        VPMMCInUse %d, \
        RTCompressible %d, \
        RTCompressMode %d, \
        PrimaryCompressible %d, \
        PrimaryCompressMode %d, \
        CompositionMode %d, \
        sfcLinearOutputByTileConvert %d",
        m_features.outputPipeMode,
        m_features.hdrMode,
        m_features.veFeatureInUse,
        m_features.scalingMode,
        m_features.deinterlaceMode,
        m_features.vpMMCInUse,
        m_features.rtCompressible,
        m_features.rtCompressMode,
        m_features.primaryCompressible,
        m_features.primaryCompressMode,
        m_features.compositionMode,
        m_features.sfcLinearOutputByTileConvert);

    if (traceEvent)
    {
        MT_LOG5(MT_VP_FTR_REPORT, MT_NORMAL, MT_VP_RENDERDATA_OUTPUT_PIPE, m_features.outputPipeMode, MT_VP_RENDER_VE_HDRMODE, m_features.hdrMode, 
        MT_VP_RENDER_VE_FTRINUSE, m_features.veFeatureInUse, MT_VP_HAL_SCALING_MODE, m_features.scalingMode, MT_VP_HAL_MMCINUSE, m_features.vpMMCInUse);
    }
    return;
}
