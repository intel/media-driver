/*
* Copyright (c) 2018-2020, Intel Corporation
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
#include "vp_vebox_cmd_packet.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"

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

    if (m_executeCaps.bSfcCsc)
    {
        VP_RENDER_CHK_STATUS_RETURN(CalculateSfcEngineParams());
    }
    else if (m_executeCaps.bBeCSC)
    {
        VP_RENDER_CHK_STATUS_RETURN(CalculateVeboxEngineParams());
    }
    else if (m_executeCaps.bRender)
    {
        // place hold for Render solution
        VP_PUBLIC_ASSERTMESSAGE("No function support CSC in Render path now");
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Error call, No function support CSC with such config");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_FORMAT GetSfcInputFormat(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat)
{
    if (executeCaps.bIECP)
    {
        // Upsampling to yuv444 for IECP input/output.
        return Format_AYUV;
    }
    else if (executeCaps.bDI && VpHal_GetSurfaceColorPack(inputFormat) == VPHAL_COLORPACK_420)
    {
        // If the input is 4:2:0, then chroma data is doubled vertically to 4:2:2
        return Format_YUY2;
    }

    return inputFormat;
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

    m_sfcCSCParams->inputColorSpcase = m_cscParams.colorSpaceInput;

    m_cscParams.formatInput         = GetSfcInputFormat(m_executeCaps, m_cscParams.formatInput);
    m_sfcCSCParams->inputFormat     = m_cscParams.formatInput;
    m_sfcCSCParams->outputFormat    = m_cscParams.formatOutput;

    if (m_sfcCSCParams->inputColorSpcase != m_cscParams.colorSpaceOutput)
    {
        m_sfcCSCParams->bCSCEnabled = true;
    }

    if (IS_RGB_CSPACE(m_cscParams.colorSpaceInput))
    {
        m_sfcCSCParams->bInputColorSpace = true;
    }
    else
    {
        m_sfcCSCParams->bInputColorSpace = false;
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

    m_veboxCSCParams->inputColorSpcase  = m_cscParams.colorSpaceInput;
    m_veboxCSCParams->outputColorSpcase = m_cscParams.colorSpaceOutput;
    m_veboxCSCParams->inputFormat       = m_cscParams.formatInput;
    m_veboxCSCParams->outputFormat      = m_cscParams.formatOutput;

    m_veboxCSCParams->bCSCEnabled = (m_cscParams.colorSpaceInput != m_cscParams.colorSpaceOutput);
    m_veboxCSCParams->alphaParams = m_cscParams.pAlphaParams;

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
    VP_RENDER_CHK_STATUS_RETURN(UpdateSfcChromaSiting());

    // Setup General params
    // Set chroma subsampling type according to the Vebox output, but
    // when Vebox is bypassed, set it according to the source surface format.

    if (VpHal_GetSurfaceColorPack(m_sfcCSCParams->inputFormat) == VPHAL_COLORPACK_444)
    {
        m_sfcCSCParams->b8tapChromafiltering = true;
    }
    else
    {
        m_sfcCSCParams->b8tapChromafiltering = false;
    }

    m_sfcCSCParams->chromaDownSamplingHorizontalCoef    = (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_HORZ_CENTER) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                        ((m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_HORZ_RIGHT) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                        MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);
    m_sfcCSCParams->chromaDownSamplingVerticalCoef      = (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_VERT_CENTER) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                        ((m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_VERT_BOTTOM) ? MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                        MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    m_sfcCSCParams->bChromaUpSamplingEnable = IsChromaUpSamplingNeeded();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::SetVeboxCUSChromaParams(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_veboxCSCParams);

    VPHAL_COLORPACK       srcColorPack;
    bool bNeedUpSampling = vpExecuteCaps.bIECP;
    bool bDIEnabled      = vpExecuteCaps.bDI;

    srcColorPack = VpHal_GetSurfaceColorPack(m_cscParams.formatInput);

    // Init CUS as disabled
    m_veboxCSCParams->bypassCUS = true;

    if (bNeedUpSampling)
    {
        // Type 0
        if ((m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_HORZ_LEFT) &&
            (m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_VERT_CENTER))
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
        else if ((m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_VERT_CENTER))
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
        else if ((m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_HORZ_LEFT) &&
                 (m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_VERT_TOP))
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
        else if ((m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_VERT_TOP))
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
        else if ((m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_HORZ_LEFT) &&
                 (m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_VERT_BOTTOM))
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
        else if ((m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.chromaSitingInput & MHW_CHROMA_SITING_VERT_BOTTOM))
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
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::SetVeboxCDSChromaParams(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    bool bNeedDownSampling = false;

    // Only VEBOX output, we use VEO to do downsampling.
    // Else, we use SFC/FC path to do downscaling.
    // if VEBOX intermediate buffer format is non_YUY2 on DI case, enable downsampling as center-left
    if (vpExecuteCaps.bDI && (m_cscParams.formatOutput != Format_YUY2))
    {
        bNeedDownSampling = true;
    }
    else
    {
        bNeedDownSampling = vpExecuteCaps.bVebox && !vpExecuteCaps.bSFC;
    }


    VPHAL_COLORPACK dstColorPack;
    dstColorPack = VpHal_GetSurfaceColorPack(m_cscParams.formatOutput);

    // Init as CDS disabled
    m_veboxCSCParams->bypassCDS = true;

    if (bNeedDownSampling)
    {
        // Type 0
        if ((m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_HORZ_LEFT) &&
            (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_VERT_CENTER))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_VERT_OFFSET;
            }
        }
        // Type 1
        else if ((m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_VERT_CENTER))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_VERT_OFFSET;
            }
        }
        // Type 2
        else if ((m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_HORZ_LEFT) &&
                 (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_VERT_TOP))
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
        else if ((m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_VERT_TOP))
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
        else if ((m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_HORZ_LEFT) &&
                 (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_VERT_BOTTOM))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_VERT_OFFSET;
            }
        }
        // Type 5
        else if ((m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_HORZ_CENTER) &&
                 (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_VERT_BOTTOM))
        {
            if (dstColorPack == VPHAL_COLORPACK_420)
            {
                m_veboxCSCParams->bypassCDS = false;
                m_veboxCSCParams->chromaDownSamplingHorizontalCoef = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_HORZ_OFFSET;
                m_veboxCSCParams->chromaDownSamplingVerticalCoef   = VP_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_VERT_OFFSET;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::UpdateSfcChromaSiting()
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_sfcCSCParams);

    switch (VpHal_GetSurfaceColorPack(m_cscParams.formatInput))
    {
    case VPHAL_COLORPACK_422:
        m_cscParams.chromaSitingInput = (m_cscParams.chromaSitingInput & 0x7) | CHROMA_SITING_VERT_TOP;
        break;
    case VPHAL_COLORPACK_444:
        m_cscParams.chromaSitingInput = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP;
        break;
    default:
        if (MHW_CHROMA_SITING_NONE == m_cscParams.chromaSitingInput)
        {
            m_cscParams.chromaSitingInput = (CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_CENTER);
        }
        break;
    }

    m_sfcCSCParams->sfcSrcChromaSiting = m_cscParams.chromaSitingInput;

    switch (VpHal_GetSurfaceColorPack(m_cscParams.formatOutput))
    {
    case VPHAL_COLORPACK_422:
        m_cscParams.chromaSitingOutput = (m_cscParams.chromaSitingOutput & 0x7) | CHROMA_SITING_VERT_TOP;
        break;
    case VPHAL_COLORPACK_444:
        m_cscParams.chromaSitingOutput = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP;
        break;
    default:
        // Prevent invalid input for output surface and format
        if (MHW_CHROMA_SITING_NONE == m_cscParams.chromaSitingOutput)
        {
            m_cscParams.chromaSitingOutput = (CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_CENTER);
        }
        break;
    }

    return MOS_STATUS_SUCCESS;
}

bool VpCscFilter::IsChromaUpSamplingNeeded()
{
    bool                  bChromaUpSampling = false;
    VPHAL_COLORPACK       srcColorPack, dstColorPack;

    srcColorPack = VpHal_GetSurfaceColorPack(m_cscParams.formatInput);
    dstColorPack = VpHal_GetSurfaceColorPack(m_cscParams.formatOutput);

    if ((srcColorPack == VPHAL_COLORPACK_420 &&
        (dstColorPack == VPHAL_COLORPACK_422 || dstColorPack == VPHAL_COLORPACK_444)) ||
        (srcColorPack == VPHAL_COLORPACK_422 && dstColorPack == VPHAL_COLORPACK_444))
    {
        bChromaUpSampling = true;
    }

    return bChromaUpSampling;
}

/****************************************************************************************************/
/*                                   HwFilter Csc Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterCscParameter::Create(HW_FILTER_CSC_PARAM &param, FeatureType featureType)
{
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
    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterCscParameter::Initialize(HW_FILTER_CSC_PARAM &param)
{
    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Sfc Csc Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpSfcCscParameter::Create(HW_FILTER_CSC_PARAM &param)
{
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

bool VpSfcCscParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VpVeboxCmdPacket *pVeboxPacket = dynamic_cast<VpVeboxCmdPacket *>(pPacket);
    if (nullptr == pVeboxPacket)
    {
        return false;
    }

    SFC_CSC_PARAMS *pParams = m_CscFilter.GetSfcParams();
    if (nullptr == pParams)
    {
        return false;
    }
    return MOS_SUCCEEDED(pVeboxPacket->SetSfcCSCParams(pParams));
}

MOS_STATUS VpSfcCscParameter::Initialize(HW_FILTER_CSC_PARAM &params)
{
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.SetExecuteEngineCaps(params.cscParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Sfc Csc Handler                                         */
/****************************************************************************************************/
PolicySfcCscHandler::PolicySfcCscHandler()
{
    m_Type = FeatureTypeCscOnSfc;
}
PolicySfcCscHandler::~PolicySfcCscHandler()
{
}

bool PolicySfcCscHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    return vpExecuteCaps.bSfcCsc;
}

HwFilterParameter *PolicySfcCscHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
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
VpPacketParameter* VpVeboxCscParameter::Create(HW_FILTER_CSC_PARAM& param)
{
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
    VpVeboxCmdPacket* pVeboxPacket = dynamic_cast<VpVeboxCmdPacket*>(pPacket);
    if (nullptr == pVeboxPacket)
    {
        return false;
    }

    VEBOX_CSC_PARAMS* pParams = m_CscFilter.GetVeboxParams();
    if (nullptr == pParams)
    {
        return false;
    }
    return MOS_SUCCEEDED(pVeboxPacket->SetVeboxBeCSCParams(pParams));
}
MOS_STATUS VpVeboxCscParameter::Initialize(HW_FILTER_CSC_PARAM& params)
{
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.SetExecuteEngineCaps(params.cscParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}
PolicyVeboxCscHandler::PolicyVeboxCscHandler()
{
    m_Type = FeatureTypeCscOnVebox;
}
PolicyVeboxCscHandler::~PolicyVeboxCscHandler()
{
}
bool PolicyVeboxCscHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    return vpExecuteCaps.bBeCSC;
}
HwFilterParameter* PolicyVeboxCscHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe& swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
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
}
