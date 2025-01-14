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
//! \file     vp_render_sfc_m12.cpp
//! \brief    SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#include "vp_render_sfc_xe_xpm_base.h"
#include "mhw_sfc_xe_xpm.h"
#include "mos_defs.h"

using namespace vp;

SfcRenderXe_Xpm_Base::SfcRenderXe_Xpm_Base(
    VP_MHWINTERFACE &vpMhwinterface,
    PVpAllocator &allocator,
    bool disbaleSfcDithering):
    SfcRenderM12(vpMhwinterface, allocator, disbaleSfcDithering)
{
}

SfcRenderXe_Xpm_Base::~SfcRenderXe_Xpm_Base()
{
    FreeResources();
}

MOS_STATUS SfcRenderXe_Xpm_Base::InitSfcStateParams()
{
    VP_FUNC_CALL();

    if (nullptr == m_sfcStateParamsLegacy)
    {
        m_sfcStateParamsLegacy = (MHW_SFC_STATE_PARAMS_XE_XPM*)MOS_AllocAndZeroMemory(sizeof(MHW_SFC_STATE_PARAMS_XE_XPM));
    }
    else
    {
        MOS_ZeroMemory(m_sfcStateParamsLegacy, sizeof(MHW_SFC_STATE_PARAMS_XE_XPM));
    }

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcStateParamsLegacy);

    m_renderDataLegacy.sfcStateParams = m_sfcStateParamsLegacy;

    return MOS_STATUS_SUCCESS;
}

bool SfcRenderXe_Xpm_Base::IsVdboxSfcInputFormatSupported(
    CODECHAL_STANDARD           codecStandard,
    MOS_FORMAT                  inputFormat)
{
    VP_FUNC_CALL();

    if (CODECHAL_JPEG == codecStandard)
    {
        if ((inputFormat != Format_NV12) &&
            (inputFormat != Format_400P) &&
            (inputFormat != Format_411P) &&
            (inputFormat != Format_IMC3) &&
            (inputFormat != Format_422H) &&
            (inputFormat != Format_444P) &&
            (inputFormat != Format_P010))
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Input Format '0x%08x' for SFC.", inputFormat);
            MT_ERR1(MT_VP_HAL_RENDER_SFC, MT_SURF_MOS_FORMAT, inputFormat);            
            return false;
        }

        return true;
    }
    else
    {
        return SfcRenderM12::IsVdboxSfcInputFormatSupported(codecStandard, inputFormat);
    }
}

bool SfcRenderXe_Xpm_Base::IsVdboxSfcOutputFormatSupported(
    CODECHAL_STANDARD           codecStandard,
    MOS_FORMAT                  outputFormat,
    MOS_TILE_TYPE               tileType)
{
    VP_FUNC_CALL();

    if (CODECHAL_JPEG == codecStandard)
    {
        if (outputFormat != Format_A8R8G8B8 &&
            outputFormat != Format_NV12 &&
            outputFormat != Format_P010 &&
            outputFormat != Format_YUY2)
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for SFC.", outputFormat);
            MT_ERR1(MT_VP_HAL_RENDER_SFC, MT_SURF_MOS_FORMAT, outputFormat);
            return false;
        }
        return true;
    }
    else
    {
        return SfcRenderM12::IsVdboxSfcOutputFormatSupported(codecStandard, outputFormat, tileType);
    }
}

MOS_STATUS SfcRenderXe_Xpm_Base::AllocateResources()
{
    VP_FUNC_CALL();

    bool                            allocated;
    PVP_SURFACE                     pTarget;
    PMHW_SFC_STATE_PARAMS_XE_XPM pSfcStateParam;

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(m_sfcStateParamsLegacy);
    VP_RENDER_CHK_NULL_RETURN(m_renderDataLegacy.pSfcPipeOutSurface);

    allocated = false;
    pTarget   = m_renderDataLegacy.pSfcPipeOutSurface;
    pSfcStateParam = (PMHW_SFC_STATE_PARAMS_XE_XPM)m_sfcStateParamsLegacy;

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBaseLegacy::AllocateResources());

    // Allocate bottom field surface for interleaved to field
    if (pSfcStateParam->iScalingType == ISCALING_INTERLEAVED_TO_FIELD)
    {
        VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
                                    m_tempFieldSurface,
                                    "OutputBottomFieldSurface",
                                    pTarget->osSurface->Format,
                                    MOS_GFXRES_2D,
                                    pTarget->osSurface->TileType,
                                    pTarget->osSurface->dwWidth,
                                    pTarget->osSurface->dwHeight,
                                    pTarget->osSurface->bIsCompressed,
                                    pTarget->osSurface->CompressionMode,
                                    allocated));
        VP_RENDER_CHK_NULL_RETURN(m_tempFieldSurface);
        VP_RENDER_CHK_NULL_RETURN(m_tempFieldSurface->osSurface);

        pSfcStateParam->tempFieldResource = &m_tempFieldSurface->osSurface->OsResource;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe_Xpm_Base::FreeResources()
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_allocator);

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBaseLegacy::FreeResources());

    // Free bottom field surface for interleaved to field
    m_allocator->DestroyVpSurface(m_tempFieldSurface);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe_Xpm_Base::SetScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBaseLegacy::SetScalingParams(scalingParams));
    VP_RENDER_CHK_STATUS_RETURN(SetInterlacedScalingParams(scalingParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe_Xpm_Base::SetCSCParams(PSFC_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBaseLegacy::SetCSCParams(cscParams));

    MHW_SFC_STATE_PARAMS_XE_XPM *param = (MHW_SFC_STATE_PARAMS_XE_XPM *)m_renderDataLegacy.sfcStateParams;
    if (cscParams->isDitheringNeeded && !m_disableSfcDithering)
    {
        param->ditheringEn = true;
    }
    else
    {
        param->ditheringEn = false;
    }
    VP_PUBLIC_NORMALMESSAGE("cscParams.isDitheringNeeded = %d, m_disableSfcDithering = %d, ditheringEn = %d",
        cscParams->isDitheringNeeded,
        m_disableSfcDithering,
        param->ditheringEn);
    return MOS_STATUS_SUCCESS;
}


MOS_STATUS SfcRenderXe_Xpm_Base::SetInterlacedScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_FUNC_CALL();

    PMHW_SFC_STATE_PARAMS_XE_XPM pSfcStateParam = nullptr;
    VP_RENDER_CHK_NULL_RETURN(scalingParams);

    pSfcStateParam = (PMHW_SFC_STATE_PARAMS_XE_XPM)m_sfcStateParamsLegacy;
    // Set interlaced scaling parameters
    if (scalingParams->interlacedScalingType != ISCALING_NONE)
    {
        pSfcStateParam->dwOutputFrameWidth  = pSfcStateParam->dwScaledRegionWidth;
        pSfcStateParam->dwOutputFrameHeight = pSfcStateParam->dwScaledRegionHeight;
    }
    pSfcStateParam->iScalingType = scalingParams->interlacedScalingType;
    switch (scalingParams->interlacedScalingType)
    {
    case ISCALING_INTERLEAVED_TO_INTERLEAVED:
        pSfcStateParam->inputFrameDataFormat  = FRAME_FORMAT_INTERLEAVED;
        pSfcStateParam->outputFrameDataFormat = FRAME_FORMAT_INTERLEAVED;
        // bottom field scaling offset
        pSfcStateParam->bottomFieldVerticalScalingOffset = MOS_UF_ROUND(1.0F / 2.0F * (1.0F / pSfcStateParam->fAVSYScalingRatio - 1.0F));
        break;
    case ISCALING_INTERLEAVED_TO_FIELD:
        pSfcStateParam->inputFrameDataFormat  = FRAME_FORMAT_INTERLEAVED;
        pSfcStateParam->outputFrameDataFormat = FRAME_FORMAT_FIELD;
        pSfcStateParam->outputSampleType      = scalingParams->dstSampleType;
        break;
    case ISCALING_FIELD_TO_INTERLEAVED:
        pSfcStateParam->inputFrameDataFormat  = FRAME_FORMAT_FIELD;
        pSfcStateParam->outputFrameDataFormat = FRAME_FORMAT_INTERLEAVED;
        if (scalingParams->srcSampleType == SAMPLE_SINGLE_TOP_FIELD)
        {
            pSfcStateParam->topBottomField = VPHAL_TOP_FIELD;
            if (scalingParams->dstSampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)
            {
                pSfcStateParam->topBottomFieldFirst = VPHAL_TOP_FIELD_FIRST;
            }
            else
            {
                pSfcStateParam->topBottomFieldFirst = VPHAL_BOTTOM_FIELD_FIRST;
            }
        }
        else
        {
            pSfcStateParam->topBottomField = VPHAL_BOTTOM_FIELD;
            if (scalingParams->dstSampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)
            {
                pSfcStateParam->topBottomFieldFirst = VPHAL_TOP_FIELD_FIRST;
            }
            else
            {
                pSfcStateParam->topBottomFieldFirst = VPHAL_BOTTOM_FIELD_FIRST;
            }
        }
        break;
    case ISCALING_FIELD_TO_FIELD:
    case ISCALING_NONE:
    default:
        pSfcStateParam->inputFrameDataFormat  = FRAME_FORMAT_PROGRESSIVE;
        pSfcStateParam->outputFrameDataFormat = FRAME_FORMAT_PROGRESSIVE;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

bool SfcRenderXe_Xpm_Base::IsOutputChannelSwapNeeded(MOS_FORMAT outputFormat)
{
    VP_FUNC_CALL();

    // ARGB8,ABGR10,A16B16G16R16,VYUY and YVYU output format need to enable swap
    if (outputFormat == Format_X8R8G8B8     ||
        outputFormat == Format_A8R8G8B8     ||
        outputFormat == Format_R10G10B10A2  ||
        outputFormat == Format_A16B16G16R16 ||
        outputFormat == Format_VYUY         ||
        outputFormat == Format_YVYU)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SfcRenderXe_Xpm_Base::IsCscNeeded(SFC_CSC_PARAMS &cscParams)
{
    VP_FUNC_CALL();

    if (m_bVdboxToSfc && cscParams.inputFormat != cscParams.outputFormat)
    {
        if (m_videoConfig.codecStandard == CODECHAL_JPEG || cscParams.outputFormat == Format_A8R8G8B8)
        {
            return true;
        }
    }
    return cscParams.bCSCEnabled || IsInputChannelSwapNeeded(cscParams.inputFormat);
}

MOS_STATUS SfcRenderXe_Xpm_Base::SetMmcParams(PMOS_SURFACE renderTarget, bool isFormatMmcSupported, bool isMmcEnabled)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderDataLegacy.sfcStateParams);

    if (((renderTarget->Format == Format_A16R16G16B16) ||
        (renderTarget->Format == Format_A16B16G16R16)) &&
        renderTarget->CompressionMode == MOS_MMC_RC)
    {
        m_renderDataLegacy.sfcStateParams->bMMCEnable = true;
        m_renderDataLegacy.sfcStateParams->MMCMode    = MOS_MMC_RC;
        VP_RENDER_NORMALMESSAGE("renderTarget->Format % d, m_renderDataLegacy.sfcStateParams->MMCMode % d", renderTarget->Format, m_renderDataLegacy.sfcStateParams->MMCMode);
        return MOS_STATUS_SUCCESS;
    }

    if (!isFormatMmcSupported && renderTarget->bIsCompressed && renderTarget->CompressionMode == MOS_MMC_RC)
    {
        VP_RENDER_ASSERTMESSAGE("CCS not cleaned due to fix function not supported the Format when compression on.");
    }

    return SfcRenderM12::SetMmcParams(renderTarget, isFormatMmcSupported, isMmcEnabled);
}
