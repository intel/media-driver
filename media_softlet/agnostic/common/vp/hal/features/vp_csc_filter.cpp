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
//! \file     vp_csc_filter.h
//! \brief    Defines the common interface for CSC
//!           this file is for the base interface which is shared by all CSC in driver.
//!

#include "vp_csc_filter.h"
#include "vp_vebox_cmd_packet_base.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_hal_ddi_utils.h"

namespace vp {

//!
//! \brief Chroma Downsampling and Upsampling for CNL+
//!
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE0_HORZ_OFFSET     0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE1_HORZ_OFFSET     1
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE2_HORZ_OFFSET     0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE3_HORZ_OFFSET     1
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE4_HORZ_OFFSET     0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE5_HORZ_OFFSET     1
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE0_VERT_OFFSET     2
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE1_VERT_OFFSET     2
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE2_VERT_OFFSET     0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE3_VERT_OFFSET     0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE4_VERT_OFFSET     4
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE5_VERT_OFFSET     4
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE0_HORZ_OFFSET  0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE1_HORZ_OFFSET  1
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE2_HORZ_OFFSET  0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE3_HORZ_OFFSET  1
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE4_HORZ_OFFSET  0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE5_HORZ_OFFSET  1
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE0_VERT_OFFSET  1
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE1_VERT_OFFSET  1
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE2_VERT_OFFSET  0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE3_VERT_OFFSET  0
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE4_VERT_OFFSET  2
#define VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE5_VERT_OFFSET  2
#define VP_VEBOX_CHROMA_UPSAMPLING_422_TYPE2_HORZ_OFFSET             0
#define VP_VEBOX_CHROMA_UPSAMPLING_422_TYPE3_HORZ_OFFSET             1
#define VP_VEBOX_CHROMA_UPSAMPLING_422_TYPE2_VERT_OFFSET             0
#define VP_VEBOX_CHROMA_UPSAMPLING_422_TYPE3_VERT_OFFSET             0
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_HORZ_OFFSET           0
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_HORZ_OFFSET           1
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE2_HORZ_OFFSET           0
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE3_HORZ_OFFSET           1
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_HORZ_OFFSET           0
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_HORZ_OFFSET           1
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_VERT_OFFSET           1
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_VERT_OFFSET           1
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE2_VERT_OFFSET           0
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE3_VERT_OFFSET           0
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_VERT_OFFSET           2
#define VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_VERT_OFFSET           2
#define VP_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE2_HORZ_OFFSET           0
#define VP_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE3_HORZ_OFFSET           1
#define VP_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE2_VERT_OFFSET           0
#define VP_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE3_VERT_OFFSET           0

MOS_FORMAT GetSfcInputFormat(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat, VPHAL_CSPACE colorSpaceOutput, MOS_FORMAT outputFormat);
VPHAL_CSPACE GetDemosaicOutputColorSpace(VPHAL_CSPACE colorSpace);
bool IsBeCscNeededForAlphaFill(MOS_FORMAT formatInput, MOS_FORMAT formatOutput, PVPHAL_ALPHA_PARAMS compAlpha);

VpCscFilter::VpCscFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{
}

MOS_STATUS VpCscFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_sfcCSCParams)
    {
        MOS_FreeMemory(m_sfcCSCParams);
        m_sfcCSCParams = nullptr;
    }

    if (m_veboxCSCParams)
    {
        MOS_FreeMemory(m_veboxCSCParams);
        m_veboxCSCParams = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::SetExecuteEngineCaps(
    FeatureParamCsc &cscParams,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_cscParams = cscParams;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();

    if (FeatureTypeCscOnSfc == m_cscParams.type)
    {
        VP_RENDER_CHK_STATUS_RETURN(CalculateSfcEngineParams());
    }
    else if (FeatureTypeCscOnVebox == m_cscParams.type)
    {
        VP_RENDER_CHK_STATUS_RETURN(CalculateVeboxEngineParams());
    }
    else if (FeatureTypeCscOnRender == m_cscParams.type)
    {
        // place hold for Render solution
        VP_PUBLIC_ASSERTMESSAGE("No function support CSC in Render path now");
        return MOS_STATUS_UNIMPLEMENTED;
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Error call, No function support CSC with such config");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

VPHAL_CSPACE GetSfcInputColorSpace(VP_EXECUTE_CAPS &executeCaps, VPHAL_CSPACE inputColorSpace, VPHAL_CSPACE colorSpaceOutput, MOS_FORMAT outputFormat)
{
    VP_FUNC_CALL();

    if (executeCaps.b3DlutOutput)
    {
        if (IS_RGB64_FLOAT_FORMAT(outputFormat)) // SFC output FP16, BT2020->BT709
        {
            return CSpace_BT2020_RGB;
        }
        else
        {
            return IS_COLOR_SPACE_BT2020(colorSpaceOutput) ? CSpace_BT2020_RGB : CSpace_sRGB;
        }
    }

    // return sRGB as colorspace as Vebox will do Bt202 to sRGB Gamut switch
    if (executeCaps.bIECP && executeCaps.bCGC && executeCaps.bBt2020ToRGB)
    {
        return CSpace_sRGB;
    }

    if (executeCaps.bDemosaicInUse)
    {
        return GetDemosaicOutputColorSpace(colorSpaceOutput);
    }
    return inputColorSpace;
}

bool VpCscFilter::IsDitheringNeeded(MOS_FORMAT formatInput, MOS_FORMAT formatOutput)
{
    uint32_t inputBitDepth = VpHalDDIUtils::GetSurfaceBitDepth(formatInput);
    if (inputBitDepth == 0)
    {
        VP_PUBLIC_ASSERTMESSAGE("Unknown Input format %d for bit depth, return false", formatInput);
        return false;
    }
    uint32_t outputBitDepth = VpHalDDIUtils::GetSurfaceBitDepth(formatOutput);
    if (outputBitDepth == 0)
    {
        VP_PUBLIC_ASSERTMESSAGE("Unknown Output format %d for bit depth, return false", formatOutput);
        return false;
    }
    if (inputBitDepth > outputBitDepth)
    {
        VP_RENDER_NORMALMESSAGE("inputFormat = %d, inputBitDepth = %d, outputFormat = %d, outputBitDepth = %d, return true",
            formatInput,
            inputBitDepth,
            formatOutput,
            outputBitDepth);
        return true;
    }
    else
    {
        VP_RENDER_NORMALMESSAGE("inputFormat = %d, inputBitDepth = %d, outputFormat = %d, outputBitDepth = %d, return false",
            formatInput,
            inputBitDepth,
            formatOutput,
            outputBitDepth);
        return false;
    }
}

MOS_STATUS VpCscFilter::CalculateSfcEngineParams()
{
    VP_FUNC_CALL();

    if (!m_executeCaps.bSFC)
    {
        VP_PUBLIC_ASSERTMESSAGE("Error call, function only support SFC CSC");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (!m_sfcCSCParams)
    {
        m_sfcCSCParams = (PSFC_CSC_PARAMS)MOS_AllocAndZeroMemory(sizeof(SFC_CSC_PARAMS));

        if (m_sfcCSCParams == nullptr)
        {
            VP_PUBLIC_ASSERTMESSAGE("sfc CSC Pamas buffer allocate failed, return nullpointer");
            return MOS_STATUS_NO_SPACE;
        }
    }
    else
    {
        MOS_ZeroMemory(m_sfcCSCParams, sizeof(SFC_CSC_PARAMS));
    }

    m_sfcCSCParams->bIEFEnable = (m_cscParams.pIEFParams &&
        m_cscParams.pIEFParams->bEnabled &&
        m_cscParams.pIEFParams->fIEFFactor > 0.0F) ? true : false;

    if (m_sfcCSCParams->bIEFEnable)
    {
        m_sfcCSCParams->iefParams = m_cscParams.pIEFParams;
    }

    m_sfcCSCParams->inputColorSpace = m_cscParams.input.colorSpace;

    // IsDitheringNeeded should be called before input format being updated by GetSfcInputFormat
    m_sfcCSCParams->isDitheringNeeded = IsDitheringNeeded(m_cscParams.formatInput, m_cscParams.formatOutput);

    m_sfcCSCParams->inputColorSpace = GetSfcInputColorSpace(m_executeCaps, m_cscParams.input.colorSpace, m_cscParams.output.colorSpace, m_cscParams.formatOutput);

    m_cscParams.formatInput         = GetSfcInputFormat(m_executeCaps, m_cscParams.formatInput, m_cscParams.output.colorSpace, m_cscParams.formatOutput);
    m_sfcCSCParams->inputFormat     = m_cscParams.formatInput;
    m_sfcCSCParams->outputFormat    = m_cscParams.formatOutput;
    m_sfcCSCParams->isFullRgbG10P709 = m_cscParams.isFullRgbG10P709;
    m_sfcCSCParams->isDemosaicNeeded = m_executeCaps.bDemosaicInUse;

    // No need to check m_cscParams.pAlphaParams as CalculateVeboxEngineParams does, as alpha is done by scaling filter on SFC.
    if (m_sfcCSCParams->inputColorSpace != m_cscParams.output.colorSpace && !(IS_RGB64_FLOAT_FORMAT(m_sfcCSCParams->outputFormat) && m_sfcCSCParams->isFullRgbG10P709))
    {
        m_sfcCSCParams->bCSCEnabled = true;
    }

    if (IS_RGB_CSPACE(m_sfcCSCParams->inputColorSpace) || IS_COLOR_SPACE_BT2020_RGB(m_sfcCSCParams->inputColorSpace))
    {
        m_sfcCSCParams->isInputColorSpaceRGB = true;
    }
    else
    {
        m_sfcCSCParams->isInputColorSpaceRGB = false;
    }

    // Set Chromasting Params
    VP_RENDER_CHK_STATUS_RETURN(SetSfcChromaParams(m_executeCaps));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::CalculateVeboxEngineParams()
{
    VP_FUNC_CALL();

    if (!m_executeCaps.bVebox)
    {
        VP_PUBLIC_ASSERTMESSAGE("Error call, function only support Vebox CSC");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (!m_veboxCSCParams)
    {
        m_veboxCSCParams = (PVEBOX_CSC_PARAMS)MOS_AllocAndZeroMemory(sizeof(VEBOX_CSC_PARAMS));

        if (m_veboxCSCParams == nullptr)
        {
            VP_PUBLIC_ASSERTMESSAGE("sfc CSC Pamas buffer allocate failed, return nullpointer");
            return MOS_STATUS_NO_SPACE;
        }
    }
    else
    {
        MOS_ZeroMemory(m_veboxCSCParams, sizeof(VEBOX_CSC_PARAMS));
    }

    bool isBeCscNeededForAlphaFill = IsBeCscNeededForAlphaFill(
        m_cscParams.formatInput, m_cscParams.formatOutput, m_cscParams.pAlphaParams);

    m_veboxCSCParams->blockType = m_executeCaps.bFeCSC ? VEBOX_CSC_BLOCK_TYPE::FRONT_END : (m_executeCaps.bBeCSC ? VEBOX_CSC_BLOCK_TYPE::BACK_END : VEBOX_CSC_BLOCK_TYPE::DEFAULT);
    m_veboxCSCParams->inputColorSpace   = m_cscParams.input.colorSpace;
    m_veboxCSCParams->outputColorSpace  = m_cscParams.output.colorSpace;
    m_veboxCSCParams->inputFormat       = m_cscParams.formatInput;
    m_veboxCSCParams->outputFormat      = m_cscParams.formatOutput;

    m_veboxCSCParams->bCSCEnabled = (m_cscParams.input.colorSpace != m_cscParams.output.colorSpace || isBeCscNeededForAlphaFill);
    m_veboxCSCParams->alphaParams = m_cscParams.pAlphaParams;

    VP_RENDER_CHK_STATUS_RETURN(UpdateChromaSiting(m_executeCaps));

    VP_RENDER_CHK_STATUS_RETURN(SetVeboxCUSChromaParams(m_executeCaps));
    VP_RENDER_CHK_STATUS_RETURN(SetVeboxCDSChromaParams(m_executeCaps));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::SetSfcChromaParams(
    VP_EXECUTE_CAPS         vpExecuteCaps)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_sfcCSCParams);

    // Update chroma sitting according to updated input format.
    VP_RENDER_CHK_STATUS_RETURN(UpdateChromaSiting(vpExecuteCaps));

    m_sfcCSCParams->sfcSrcChromaSiting = m_cscParams.input.chromaSiting;

    // Setup General params
    // Set chroma subsampling type according to the Vebox output, but
    // when Vebox is bypassed, set it according to the source surface format.
    // VDBOX SFC doesn't use 8 tap chroma filtering for all input format.

    if (vpExecuteCaps.bVebox)
    {
        if (VpHalDDIUtils::GetSurfaceColorPack(m_sfcCSCParams->inputFormat) == VPHAL_COLORPACK_444)
        {
            m_sfcCSCParams->b8tapChromafiltering = true;
        }
        else
        {
            m_sfcCSCParams->b8tapChromafiltering = false;
        }
    }
    else
    {
        m_sfcCSCParams->b8tapChromafiltering = false;
    }

    m_sfcCSCParams->chromaDownSamplingHorizontalCoef    = (m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                        ((m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                        MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);
    m_sfcCSCParams->chromaDownSamplingVerticalCoef      = (m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_VERT_CENTER) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                        ((m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                        MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    m_sfcCSCParams->bChromaUpSamplingEnable = IsChromaUpSamplingNeeded();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::SetVeboxCUSChromaParams(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_veboxCSCParams);

    VPHAL_COLORPACK       srcColorPack;
    bool bNeedUpSampling = vpExecuteCaps.bIECP || m_veboxCSCParams->bCSCEnabled ||
        (vpExecuteCaps.b3DlutOutput && !vpExecuteCaps.bHDR3DLUT);
    bool bDIEnabled      = vpExecuteCaps.bDI;

    if (Format_None != m_cscParams.formatforCUS)
    {
        srcColorPack = VpHalDDIUtils::GetSurfaceColorPack(m_cscParams.formatforCUS);
    }
    else
    {
        srcColorPack = VpHalDDIUtils::GetSurfaceColorPack(m_cscParams.formatInput);
    }

    // Init CUS as disabled
    m_veboxCSCParams->bypassCUS = true;

    if (bNeedUpSampling)
    {
        // Type 0
        if ((m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_HORZ_LEFT) &&
            (m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_VERT_CENTER))
        {
            if (srcColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCUS = false;
                if (bDIEnabled)
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE0_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE0_VERT_OFFSET;
                }
                else
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE0_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE0_VERT_OFFSET;
                }
            }
        }
        // Type 1
        else if ((m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_VERT_CENTER))
        {
            if (srcColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCUS = false;
                if (bDIEnabled)
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE1_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE1_VERT_OFFSET;
                }
                else
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE1_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE1_VERT_OFFSET;
                }
            }
        }
        // Type 2
        else if ((m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_HORZ_LEFT) &&
                 (m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_VERT_TOP))
        {
            if (srcColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCUS = false;
                if (bDIEnabled)
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE2_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE2_VERT_OFFSET;
                }
                else
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE2_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE2_VERT_OFFSET;
                }
            }
            else if (srcColorPack == VPHAL_COLORPACK_422)
            {
                m_veboxCSCParams->bypassCUS = false;
                m_veboxCSCParams->chromaUpSamplingHorizontalCoef     = VP_VEBOX_CHROMA_UPSAMPLING_422_TYPE2_HORZ_OFFSET;
                m_veboxCSCParams->chromaUpSamplingVerticalCoef       = VP_VEBOX_CHROMA_UPSAMPLING_422_TYPE2_VERT_OFFSET;
            }
        }
        // Type 3
        else if ((m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_VERT_TOP))
        {
            if (srcColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCUS = false;
                if (bDIEnabled)
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE3_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE3_VERT_OFFSET;
                }
                else
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE3_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE3_VERT_OFFSET;
                }
            }
            else if (srcColorPack == VPHAL_COLORPACK_422)
            {
                m_veboxCSCParams->bypassCUS = false;
                m_veboxCSCParams->chromaUpSamplingHorizontalCoef     = VP_VEBOX_CHROMA_UPSAMPLING_422_TYPE3_HORZ_OFFSET;
                m_veboxCSCParams->chromaUpSamplingVerticalCoef       = VP_VEBOX_CHROMA_UPSAMPLING_422_TYPE3_VERT_OFFSET;
            }
        }
        // Type 4
        else if ((m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_HORZ_LEFT) &&
                 (m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM))
        {
            if (srcColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCUS = false;
                if (bDIEnabled)
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE4_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE4_VERT_OFFSET;
                }
                else
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE4_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE4_VERT_OFFSET;
                }
            }
        }
        // Type 5
        else if ((m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.input.chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM))
        {
            if (srcColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCUS = false;
                if (bDIEnabled)
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE5_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE5_VERT_OFFSET;
                }
                else
                {
                    m_veboxCSCParams->chromaUpSamplingHorizontalCoef = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE5_HORZ_OFFSET;
                    m_veboxCSCParams->chromaUpSamplingVerticalCoef   = VP_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE5_VERT_OFFSET;
                }
            }
        }
    }

    VP_RENDER_NORMALMESSAGE("bypassCUS %d, chromaUpSamplingHorizontalCoef %d, chromaUpSamplingVerticalCoef %d",
        m_veboxCSCParams->bypassCUS, m_veboxCSCParams->chromaUpSamplingHorizontalCoef, m_veboxCSCParams->chromaUpSamplingVerticalCoef);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::SetVeboxCDSChromaParams(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    bool bNeedDownSampling = false;

    VPHAL_COLORPACK dstColorPack = VpHalDDIUtils::GetSurfaceColorPack(m_cscParams.formatOutput);

    // Only VEBOX output, we use VEO to do downsampling.
    // Else, we use SFC/FC path to do downscaling.
    // if VEBOX intermediate buffer format is non_YUY2 on DI case, enable downsampling as center-left
    if (vpExecuteCaps.bDI && (m_cscParams.formatOutput != Format_YUY2 || vpExecuteCaps.bIECP))
    {
        bNeedDownSampling = true;
    }
    else
    {
        bNeedDownSampling = vpExecuteCaps.bVebox && !vpExecuteCaps.bSFC && !vpExecuteCaps.bForceCscToRender;
    }

    // Init as CDS disabled
    m_veboxCSCParams->bypassCDS = true;

    if (bNeedDownSampling)
    {
        // Type 0
        if ((m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_HORZ_LEFT) &&
            (m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_VERT_CENTER))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_VERT_OFFSET;
            }
        }
        // Type 1
        else if ((m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_VERT_CENTER))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_VERT_OFFSET;
            }
        }
        // Type 2
        else if ((m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_HORZ_LEFT) &&
                 (m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_VERT_TOP))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE2_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE2_VERT_OFFSET;
            }
            else if (dstColorPack == VPHAL_COLORPACK_422)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE2_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE2_VERT_OFFSET;
            }
        }
        // Type 3
        else if ((m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_VERT_TOP))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE3_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE3_VERT_OFFSET;
            }
            else if (dstColorPack == VPHAL_COLORPACK_422)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE3_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE3_VERT_OFFSET;
            }
        }
        // Type 4
        else if ((m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_HORZ_LEFT) &&
                 (m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_VERT_OFFSET;
            }
        }
        // Type 5
        else if ((m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.output.chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_VERT_OFFSET;
            }
        }
    }

    VP_RENDER_NORMALMESSAGE("bypassCDS %d, chromaDownSamplingHorizontalCoef %d, chromaDownSamplingVerticalCoef %d",
        m_veboxCSCParams->bypassCDS, m_veboxCSCParams->chromaDownSamplingHorizontalCoef, m_veboxCSCParams->chromaDownSamplingVerticalCoef);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::UpdateChromaSiting(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    // For VDBOX input, just using the chroma siting input directly.
    if (!vpExecuteCaps.bVebox)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (MHW_CHROMA_SITING_NONE == m_cscParams.input.chromaSiting)
    {
        m_cscParams.input.chromaSiting = (CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_CENTER);
    }
    switch (VpHalDDIUtils::GetSurfaceColorPack(m_cscParams.formatInput))
    {
    case VPHAL_COLORPACK_422:
        m_cscParams.input.chromaSiting = (m_cscParams.input.chromaSiting & 0x7) | CHROMA_SITING_VERT_TOP;
        break;
    case VPHAL_COLORPACK_444:
        m_cscParams.input.chromaSiting = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP;
        break;
    default:
        break;
    }

    if (MHW_CHROMA_SITING_NONE == m_cscParams.output.chromaSiting)
    {
        m_cscParams.output.chromaSiting = (CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_CENTER);
    }
    switch (VpHalDDIUtils::GetSurfaceColorPack(m_cscParams.formatOutput))
    {
    case VPHAL_COLORPACK_422:
        m_cscParams.output.chromaSiting = (m_cscParams.output.chromaSiting & 0x7) | CHROMA_SITING_VERT_TOP;
        break;
    case VPHAL_COLORPACK_444:
        m_cscParams.output.chromaSiting = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP;
        break;
    default:
        break;
    }

    return MOS_STATUS_SUCCESS;
}

bool VpCscFilter::IsChromaUpSamplingNeeded()
{
    VP_FUNC_CALL();

    bool                  bChromaUpSampling = false;
    VPHAL_COLORPACK       srcColorPack, dstColorPack;

    srcColorPack = VpHalDDIUtils::GetSurfaceColorPack(m_cscParams.formatInput);
    dstColorPack = VpHalDDIUtils::GetSurfaceColorPack(m_cscParams.formatOutput);

    if ((srcColorPack == VPHAL_COLORPACK_420 &&
        (dstColorPack == VPHAL_COLORPACK_422 || dstColorPack == VPHAL_COLORPACK_444)) ||
        (srcColorPack == VPHAL_COLORPACK_422 && dstColorPack == VPHAL_COLORPACK_444))
    {
        bChromaUpSampling = true;
    }

    VP_PUBLIC_NORMALMESSAGE("formatInput %d, formatOutput %d, srcColorPack %d, dstColorPack %d, bChromaUpSampling %d",
        m_cscParams.formatInput, m_cscParams.formatOutput, srcColorPack, dstColorPack, bChromaUpSampling ? 1 : 0);

    return bChromaUpSampling;
}

/****************************************************************************************************/
/*                                   HwFilter Csc Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterCscParameter::Create(HW_FILTER_CSC_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterCscParameter *p = MOS_New(HwFilterCscParameter, featureType);
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            MOS_Delete(p);
            return nullptr;
        }
    }
    return p;
}

HwFilterCscParameter::HwFilterCscParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterCscParameter::~HwFilterCscParameter()
{
}

MOS_STATUS HwFilterCscParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterCscParameter::Initialize(HW_FILTER_CSC_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Sfc Csc Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpSfcCscParameter::Create(HW_FILTER_CSC_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpSfcCscParameter *p = dynamic_cast<VpSfcCscParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            VpPacketParameter *pParam = p;
            param.pPacketParamFactory->ReturnPacketParameter(pParam);
            return nullptr;
        }
    }
    return p;
}

VpSfcCscParameter::VpSfcCscParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_CscFilter(pHwInterface)
{
}
VpSfcCscParameter::~VpSfcCscParameter() {}

bool VpSfcCscParameter::SetPacketParam(VpCmdPacket *_packet)
{
    VP_FUNC_CALL();

    SFC_CSC_PARAMS *params = m_CscFilter.GetSfcParams();
    if (nullptr == params)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get sfc csc params");
        return false;
    }

    VpVeboxCmdPacketBase *packet = dynamic_cast<VpVeboxCmdPacketBase *>(_packet);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetSfcCSCParams(params));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for sfc csc");
    return false;
}

MOS_STATUS VpSfcCscParameter::Initialize(HW_FILTER_CSC_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.SetExecuteEngineCaps(params.cscParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Sfc Csc Handler                                         */
/****************************************************************************************************/
PolicySfcCscHandler::PolicySfcCscHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeCscOnSfc;
}
PolicySfcCscHandler::~PolicySfcCscHandler()
{
}

bool PolicySfcCscHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bSfcCsc;
}

HwFilterParameter *PolicySfcCscHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterCsc *swFilter = dynamic_cast<SwFilterCsc *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeCscOnSfc));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamCsc &param = swFilter->GetSwFilterParams();

        HW_FILTER_CSC_PARAM paramCsc = {};
        paramCsc.type = m_Type;
        paramCsc.pHwInterface = pHwInterface;
        paramCsc.vpExecuteCaps = vpExecuteCaps;
        paramCsc.pPacketParamFactory = &m_PacketParamFactory;
        paramCsc.cscParams = param;
        paramCsc.pfnCreatePacketParam = PolicySfcCscHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterCscParameter*)pHwFilterParam)->Initialize(paramCsc)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterCscParameter::Create(paramCsc, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicySfcCscHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    SwFilterCsc *featureCsc = dynamic_cast<SwFilterCsc *>(&feature);
    VP_PUBLIC_CHK_NULL_RETURN(featureCsc);

    if (caps.b1stPassOfSfc2PassScaling || caps.bForceCscToRender)
    {
        SwFilterCsc *filter2ndPass = featureCsc;
        SwFilterCsc *filter1ndPass = (SwFilterCsc *)feature.Clone();

        VP_PUBLIC_CHK_NULL_RETURN(filter1ndPass);
        VP_PUBLIC_CHK_NULL_RETURN(filter2ndPass);

        filter1ndPass->GetFilterEngineCaps() = filter2ndPass->GetFilterEngineCaps();
        filter1ndPass->SetFeatureType(filter2ndPass->GetFeatureType());

        FeatureParamCsc &params2ndPass = filter2ndPass->GetSwFilterParams();
        FeatureParamCsc &params1stPass = filter1ndPass->GetSwFilterParams();

        // No csc in 1st pass.
        params1stPass.formatOutput = params1stPass.formatInput;
        params1stPass.output = params1stPass.input;
        if (caps.b1stPassOfSfc2PassScaling)
        {
            params1stPass.pIEFParams = nullptr;
        }
        else
        {
            // 2nd pass is render. Do IEF in first pass as it is not supported in 3D sampler.
            params1stPass.pIEFParams = params2ndPass.pIEFParams;
            params2ndPass.pIEFParams = nullptr;
        }
        params1stPass.pAlphaParams = nullptr;

        // Clear engine caps for filter in 2nd pass.
        filter2ndPass->SetFeatureType(FeatureTypeCsc);
        filter2ndPass->GetFilterEngineCaps().usedForNextPass = 1;

        if (caps.bForceCscToRender)
        {
            // Switch to render engine for csc filter.
            VP_PUBLIC_NORMALMESSAGE("Force csc to render case. VeboxNeeded: %d -> 0, SfcNeeded: %d -> %d, RenderNeeded: %d -> 1",
                filter2ndPass->GetFilterEngineCaps().VeboxNeeded,
                filter2ndPass->GetFilterEngineCaps().SfcNeeded,
                caps.b1stPassOfSfc2PassScaling,
                filter2ndPass->GetFilterEngineCaps().RenderNeeded);
            filter2ndPass->GetFilterEngineCaps().bEnabled  = 1;
            filter2ndPass->GetFilterEngineCaps().VeboxNeeded = 0;
            filter2ndPass->GetFilterEngineCaps().SfcNeeded = caps.b1stPassOfSfc2PassScaling;
            filter2ndPass->GetFilterEngineCaps().RenderNeeded = 1;
            filter2ndPass->GetFilterEngineCaps().fcSupported = 1;
        }

        executePipe.AddSwFilterUnordered(filter1ndPass, isInputPipe, index);
    }
    else
    {
        return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }

    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Vebox Csc Parameter                                            */
/****************************************************************************************************/
VpPacketParameter* VpVeboxCscParameter::Create(HW_FILTER_CSC_PARAM& param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpVeboxCscParameter* p = dynamic_cast<VpVeboxCscParameter*>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            VpPacketParameter* pParam = p;
            param.pPacketParamFactory->ReturnPacketParameter(pParam);
            return nullptr;
        }
    }
    return p;
}
VpVeboxCscParameter::VpVeboxCscParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase* packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_CscFilter(pHwInterface)
{
}
VpVeboxCscParameter::~VpVeboxCscParameter()
{
}
bool VpVeboxCscParameter::SetPacketParam(VpCmdPacket* pPacket)
{
    VP_FUNC_CALL();

    VEBOX_CSC_PARAMS *params = m_CscFilter.GetVeboxParams();
    if (nullptr == params)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get vebox be csc params");
        return false;
    }

    VpVeboxCmdPacketBase *packet = dynamic_cast<VpVeboxCmdPacketBase *>(pPacket);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetVeboxCSCParams(params));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for vebox be csc");
    return false;
}
MOS_STATUS VpVeboxCscParameter::Initialize(HW_FILTER_CSC_PARAM& params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.SetExecuteEngineCaps(params.cscParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}
PolicyVeboxCscHandler::PolicyVeboxCscHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeCscOnVebox;
}
PolicyVeboxCscHandler::~PolicyVeboxCscHandler()
{
}
bool PolicyVeboxCscHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bBeCSC || vpExecuteCaps.bFeCSC;
}
HwFilterParameter* PolicyVeboxCscHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe& swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterCsc *swFilter = dynamic_cast<SwFilterCsc *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeCscOnVebox));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamCsc &param = swFilter->GetSwFilterParams();

        HW_FILTER_CSC_PARAM paramCsc = {};
        paramCsc.type = m_Type;
        paramCsc.pHwInterface = pHwInterface;
        paramCsc.vpExecuteCaps = vpExecuteCaps;
        paramCsc.pPacketParamFactory = &m_PacketParamFactory;
        paramCsc.cscParams = param;
        paramCsc.pfnCreatePacketParam = PolicyVeboxCscHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterCscParameter*)pHwFilterParam)->Initialize(paramCsc)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterCscParameter::Create(paramCsc, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicyVeboxCscHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    SwFilterCsc *featureCsc = dynamic_cast<SwFilterCsc *>(&feature);
    VP_PUBLIC_CHK_NULL_RETURN(featureCsc);

    if (!featureCsc->GetFilterEngineCaps().VeboxNeeded || caps.bForceCscToRender)
    {
        SwFilterCsc *filter2ndPass = featureCsc;
        SwFilterCsc *filter1ndPass = (SwFilterCsc *)feature.Clone();

        VP_PUBLIC_CHK_NULL_RETURN(filter1ndPass);
        VP_PUBLIC_CHK_NULL_RETURN(filter2ndPass);

        filter1ndPass->GetFilterEngineCaps() = filter2ndPass->GetFilterEngineCaps();

        if (!filter1ndPass->GetFilterEngineCaps().VeboxNeeded)
        {
            VP_PUBLIC_NORMALMESSAGE("VeboxNeeded being false. Just do chroma sitting in current pass");
            filter1ndPass->GetFilterEngineCaps().VeboxNeeded = 1;
        }

        filter1ndPass->SetFeatureType(filter2ndPass->GetFeatureType());

        FeatureParamCsc &params2ndPass = filter2ndPass->GetSwFilterParams();
        FeatureParamCsc &params1stPass = filter1ndPass->GetSwFilterParams();

        // No csc in 1st pass.
        params1stPass.formatOutput = params1stPass.formatInput;
        params1stPass.output       = params1stPass.input;

        // vebox + render no IEF Params
        params1stPass.pIEFParams = nullptr;
        params2ndPass.pIEFParams = nullptr;

        params1stPass.pAlphaParams = nullptr;

        // Clear engine caps for filter in 2nd pass.
        filter2ndPass->SetFeatureType(FeatureTypeCsc);
        filter2ndPass->GetFilterEngineCaps().usedForNextPass = 1;

        if (caps.bForceCscToRender)
        {
            // Switch to render engine for csc filter.
            VP_PUBLIC_NORMALMESSAGE("Force csc to render case. VeboxNeeded: %d -> 0, RenderNeeded: %d -> 1",
                filter2ndPass->GetFilterEngineCaps().VeboxNeeded,
                filter2ndPass->GetFilterEngineCaps().RenderNeeded);
            filter2ndPass->GetFilterEngineCaps().bEnabled     = 1;
            filter2ndPass->GetFilterEngineCaps().VeboxNeeded  = 0;
            filter2ndPass->GetFilterEngineCaps().RenderNeeded = 1;
            filter2ndPass->GetFilterEngineCaps().fcSupported  = 1;
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("VeboxNeeded being false.Keep engine caps for next pass no change. enable %d, SfcNeeded %d, RenderNeeded %d",
                filter2ndPass->GetFilterEngineCaps().bEnabled,
                filter2ndPass->GetFilterEngineCaps().SfcNeeded,
                filter2ndPass->GetFilterEngineCaps().RenderNeeded);
        }

        executePipe.AddSwFilterUnordered(filter1ndPass, isInputPipe, index);
    }
    else
    {
        return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }

    return MOS_STATUS_SUCCESS;
}
}
