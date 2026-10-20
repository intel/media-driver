/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     vp_hdrlite_render_filter.cpp
//! \brief    Defines the common interface for Adaptive Contrast Enhancement
//!           this file is for the base interface which is shared by all Hdr in driver.
//!
#include <array>
#include "vp_hdrlite_render_filter.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_render_cmd_packet.h"
#include "vp_pipeline.h"

namespace vp
{

void RENDER_HDRLITE_KERNEL_PARAM::Init()
{
    kernelArgs.clear();
    kernelID     = VpKernelID(vpKernelIDNextMax); 
    threadWidth  = 0;
    threadHeight = 0;
    threadDepth  = 0;
    localWidth   = 0;
    localHeight  = 0;
    kernelStatefulSurfaces.clear();
}

void _RENDER_HDRLITE_PARAMS::Init()
{
    kernelParams.clear();
}

VpHdrLiteRenderFilter::VpHdrLiteRenderFilter(PVP_MHWINTERFACE vpMhwInterface) : VpFilter(vpMhwInterface)
{
}

MOS_STATUS VpHdrLiteRenderFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::Destroy()
{
    VP_FUNC_CALL();

    for (auto &handle : m_hdrMandatoryKrnArgs)
    {
        KRN_ARG &krnArg = handle.second;
        MOS_FreeMemAndSetNull(krnArg.pData);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::SetExecuteEngineCaps(
    SwFilterPipe   *executedPipe,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executedPipe = executedPipe;
    m_executeCaps  = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::CalculateEngineParams(
    FeatureParamHdr &hdrParams,
    VP_EXECUTE_CAPS  vpExecuteCaps)
{
    VP_FUNC_CALL();
    uint32_t    i         = 0;
    VP_SURFACE *inputSrc  = nullptr;
    VP_SURFACE *targetSrc = nullptr;

    VP_PUBLIC_CHK_NULL_RETURN(m_executedPipe);
    if (!vpExecuteCaps.bRender || HDR_STAGE_3DLUT_KERNEL == hdrParams.stage)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // create a filter Param buffer
    m_renderHdrParams.Init();

    HDRLITE_HAL_PARAM param = {};
    VP_PUBLIC_CHK_STATUS_RETURN(InitHalParam(*m_executedPipe, hdrParams, param));

    // Generate HDR kernel parameters (kernel args and surfaces)
    RENDER_HDRLITE_KERNEL_PARAM renderParam = {};
    VP_PUBLIC_CHK_STATUS_RETURN(GenerateHdrKrnParam(
        param,
        m_executedPipe->GetSurfacesSetting().surfGroup,
        renderParam));

    // Add renderParam to m_renderHdrParams
    m_renderHdrParams.kernelParams.push_back(renderParam);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::InitHalParam(SwFilterPipe &executingPipe, FeatureParamHdr &hdrParams, HDRLITE_HAL_PARAM &param)
{
    param.layer = uint16_t(executingPipe.GetSurfaceCount(true));
    if (hdrParams.pColorFillParams)
    {
        param.enableColorFill = true;
        param.colorFillParams = *hdrParams.pColorFillParams;
    }
    else
    {
        param.enableColorFill = false;
    }
    param.uiMaxDisplayLum = hdrParams.uiMaxDisplayLum;

    auto &surfGroup       = executingPipe.GetSurfacesSetting().surfGroup;
    auto  targetSrcHandle = surfGroup.find(SurfaceTypeHdrTarget0);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(targetSrcHandle, &surfGroup);
    VP_PUBLIC_CHK_NULL_RETURN(targetSrcHandle->second);
    VP_PUBLIC_CHK_NULL_RETURN(targetSrcHandle->second->osSurface);

    VpAllocator &allocator = executingPipe.GetVpInterface().GetAllocator();

    bool needUpdateTarget = false;
    if (memcmp(&hdrParams.targetHDRParams, &m_lastFrameTargetParams, sizeof(HDR_PARAMS)))
    {
        needUpdateTarget        = true;
        m_lastFrameTargetParams = hdrParams.targetHDRParams;
    }

    bool hasUpdate = needUpdateTarget;

    for (uint32_t i = 0; i < param.layer && i < VPHAL_MAX_HDR_INPUT_LAYER; ++i)
    {
        bool             needUpdate  = false;

        VP_PUBLIC_CHK_STATUS_RETURN(InitLayerParam(
            executingPipe,
            i,
            hdrParams.targetHDRParams,
            targetSrcHandle->second->ColorSpace,
            targetSrcHandle->second->osSurface->Format,
            param.inputLayerParam[i],
            needUpdate));
        hasUpdate |= needUpdate;

        auto        oetf1DLUTHandle = surfGroup.find(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + i));
        VP_SURFACE *oetf1DLUTSrc    = (surfGroup.end() != oetf1DLUTHandle) ? oetf1DLUTHandle->second : nullptr;
        if (oetf1DLUTSrc && (needUpdate || needUpdateTarget || executingPipe.GetSurfacesSetting().OETF1DLUTAllocated))
        {
            VP_PUBLIC_CHK_STATUS_RETURN(InitLayerOETF1DLUT(allocator, hdrParams.targetHDRParams, param.inputLayerParam[i], oetf1DLUTSrc));
        }

        auto        cri3DLutHandle = surfGroup.find(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + i));
        VP_SURFACE *cri3DLUTSrc    = (surfGroup.end() != cri3DLutHandle) ? cri3DLutHandle->second : nullptr;
        if (cri3DLUTSrc && (needUpdate || needUpdateTarget || executingPipe.GetSurfacesSetting().Cri3DLUTAllocated))
        {
            auto inputHandle = surfGroup.find(SurfaceType(SurfaceTypeHdrInputLayer0 + i));
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(inputHandle, &surfGroup);
            VP_PUBLIC_CHK_STATUS_RETURN(InitLayerCri3DLut(allocator, hdrParams.targetHDRParams, param.inputLayerParam[i], inputHandle->second, cri3DLUTSrc));
        }
    }
    param.targetHdrParams = hdrParams.targetHDRParams;

    if (hasUpdate || executingPipe.GetSurfacesSetting().coeffAllocated) //?if need change to &&
    {
        VP_PUBLIC_CHK_STATUS_RETURN(InitCoeff(allocator, param, surfGroup));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::InitLayerParam(
    SwFilterPipe     &executingPipe,
    uint32_t          index,
    HDR_PARAMS       &targetHdrParam,
    VPHAL_CSPACE      targetCSpace,
    MOS_FORMAT        targetFormat,
    HDRLITE_LAYER_PARAM &param,
    bool             &needUpdate)
{
    HDRStageEnables &stageEnable = param.stageEnable;
    needUpdate                   = false;
    stageEnable.value            = 0;

    SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(executingPipe.GetSwFilter(true, index, FeatureTypeScaling));
    param.scalingMode        = scaling ? scaling->GetSwFilterParams().scalingMode : VPHAL_SCALING_BILINEAR;

    SwFilterRotMir *rotation = dynamic_cast<SwFilterRotMir *>(executingPipe.GetSwFilter(true, index, FeatureTypeRotMir));
    param.rotation           = rotation ? rotation->GetSwFilterParams().rotation : VPHAL_ROTATION_IDENTITY;

    auto blending = dynamic_cast<SwFilterBlending *>(executingPipe.GetSwFilter(true, index, FeatureTypeBlending));
    if (blending)
    {
        auto &blendingParams = blending->GetSwFilterParams();
        if (blendingParams.blendingParams)
        {
            param.blending = *blendingParams.blendingParams;
        }
    }

    SwFilterHdr *hdrfilter = dynamic_cast<SwFilterHdr *>(executingPipe.GetSwFilter(true, index, FeatureTypeHdr));
    if (nullptr == hdrfilter)
    {
        param.enabled = false;
        return MOS_STATUS_SUCCESS;
    }

    auto &hdrParams = hdrfilter->GetSwFilterParams();
    auto &surfGroup = executingPipe.GetSurfacesSetting().surfGroup;

    SurfaceType surfId         = (SurfaceType)(SurfaceTypeHdrInputLayer0 + index);
    auto        inputSrcHandle = surfGroup.find(surfId);
    if (inputSrcHandle == surfGroup.end() || inputSrcHandle->second == nullptr || inputSrcHandle->second->osSurface == nullptr)
    {
        param.enabled = false;
        return MOS_STATUS_SUCCESS;
    }
    PVP_SURFACE input = inputSrcHandle->second;

    param.hdrParams = hdrParams.srcHDRParams;

    //ToneMappingStagesAssemble
    const uint16_t *stageConfigTable = executingPipe.GetSurfacesSetting().pHDRStageConfigTable;
    VP_PUBLIC_CHK_NULL_RETURN(stageConfigTable);

    // Because FP16 format can represent both SDR or HDR, we need do judgement here.
    // We need this information because we dont have unified tone mapping algorithm for various scenarios(H2S/H2H).
    // To do this, we make two assumptions:
    // 1. This colorspace will be set to BT709/Gamma1.0 from APP, so such information can NOT be used to check HDR.
    // 2. If APP pass any HDR metadata, it indicates this is HDR.
    HDRCaseID id    = {0};
    id.InputXDR     = (param.hdrParams.EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084 || IS_RGB64_FLOAT_FORMAT(input->osSurface->Format));
    id.InputGamut   = IS_COLOR_SPACE_BT2020(input->ColorSpace);
    id.OutputXDR    = targetHdrParam.EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084 || IS_RGB64_FLOAT_FORMAT(targetFormat);
    id.OutputGamut  = IS_COLOR_SPACE_BT2020(targetCSpace);
    id.OutputLinear = IS_RGB64_FLOAT_FORMAT(targetFormat);

    HDRStageConfigEntry stageConfig = {};
    stageConfig.value               = stageConfigTable[id.index];
    VP_PUBLIC_CHK_VALUE_RETURN(stageConfig.Invalid, 0);

    VPHAL_HDR_MODE     currentHdrMode = (VPHAL_HDR_MODE)stageConfig.PWLF;
    VPHAL_HDR_CCM_TYPE currentCCM     = (VPHAL_HDR_CCM_TYPE)stageConfig.CCM;
    VPHAL_HDR_CCM_TYPE currentCCMExt1 = (VPHAL_HDR_CCM_TYPE)stageConfig.CCMExt1;
    VPHAL_HDR_CCM_TYPE currentCCMExt2 = (VPHAL_HDR_CCM_TYPE)stageConfig.CCMExt2;

    // So far only enable auto mode in H2S cases.
    if (currentHdrMode == VPHAL_HDR_MODE_TONE_MAPPING &&
        param.hdrParams.bAutoMode &&
        inputSrcHandle->second->SurfType == SURF_IN_PRIMARY)
    {
        currentHdrMode = VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE;
    }

    stageEnable.CCMEnable         = currentCCM != VPHAL_HDR_CCM_NONE;
    stageEnable.PWLFEnable        = currentHdrMode != VPHAL_HDR_MODE_NONE;
    stageEnable.CCMExt1Enable     = currentCCMExt1 != VPHAL_HDR_CCM_NONE;
    stageEnable.CCMExt2Enable     = currentCCMExt2 != VPHAL_HDR_CCM_NONE;
    stageEnable.GamutClamp1Enable = stageConfig.GamutClamp1;
    stageEnable.GamutClamp2Enable = stageConfig.GamutClamp2;

    VPHAL_HDR_LUT_MODE currentLUTMode  = VPHAL_HDR_LUT_MODE_NONE;
    VPHAL_GAMMA_TYPE   currentEOTF     = VPHAL_GAMMA_NONE;    //!< EOTF
    VPHAL_GAMMA_TYPE   currentOETF     = VPHAL_GAMMA_NONE;    //!< OETF
    VPHAL_HDR_CSC_TYPE currentPriorCSC = VPHAL_HDR_CSC_NONE;  //!< Prior CSC Mode
    VPHAL_HDR_CSC_TYPE currentPostCSC  = VPHAL_HDR_CSC_NONE;  //!< Post CSC Mode

    if (IS_YUV_FORMAT(input->osSurface->Format) || IS_ALPHA_YUV_FORMAT(input->osSurface->Format))
    {
        stageEnable.PriorCSCEnable = 1;
    }

    if (!IS_RGB64_FLOAT_FORMAT(input->osSurface->Format) &&
        (stageEnable.CCMEnable || stageEnable.PWLFEnable || stageEnable.CCMExt1Enable || stageEnable.CCMExt2Enable))
    {
        stageEnable.EOTFEnable = 1;
    }

    if (!IS_RGB64_FLOAT_FORMAT(targetFormat) && (stageEnable.EOTFEnable || IS_RGB64_FLOAT_FORMAT(input->osSurface->Format)))
    {
        stageEnable.OETFEnable = 1;
    }

    if (IS_YUV_FORMAT(targetFormat))
    {
        stageEnable.PostCSCEnable = 1;
    }

    if (input->SurfType == SURF_IN_PRIMARY && m_globalLutMode != VPHAL_HDR_LUT_MODE_3D)
    {
        currentLUTMode = VPHAL_HDR_LUT_MODE_2D;
    }
    else
    {
        currentLUTMode = VPHAL_HDR_LUT_MODE_3D;
    }

    // Neither 1D nor 3D LUT is needed in linear output case.
    if (IS_RGB64_FLOAT_FORMAT(targetFormat))
    {
        currentLUTMode = VPHAL_HDR_LUT_MODE_NONE;
    }

    // EOTF/CCM/Tone Mapping/OETF require RGB input
    // So if prior CSC is needed, it will always be YUV to RGB conversion
    if (stageEnable.PriorCSCEnable)
    {
        if (input->ColorSpace == CSpace_BT601)
        {
            currentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT601;
        }
        else if (input->ColorSpace == CSpace_BT709)
        {
            currentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT709;
        }
        else if (input->ColorSpace == CSpace_BT2020)
        {
            currentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT2020;
        }
        else if (input->ColorSpace == CSpace_BT2020_FullRange)
        {
            currentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT2020;
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    if (stageEnable.EOTFEnable)
    {
        if (param.hdrParams.EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR ||
            param.hdrParams.EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR)
        {
            // Mark tranditional HDR/SDR gamma as the same type
            currentEOTF = VPHAL_GAMMA_TRADITIONAL_GAMMA;
        }
        else if (param.hdrParams.EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
        {
            currentEOTF = VPHAL_GAMMA_SMPTE_ST2084;
        }
        else if (param.hdrParams.EOTF == VPHAL_HDR_EOTF_BT1886)
        {
            currentEOTF = VPHAL_GAMMA_BT1886;
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    if (stageEnable.OETFEnable)
    {
        if (targetHdrParam.EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR ||
            targetHdrParam.EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR)
        {
            currentOETF = VPHAL_GAMMA_SRGB;
        }
        else if (targetHdrParam.EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
        {
            currentOETF = VPHAL_GAMMA_SMPTE_ST2084;
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    // OETF will output RGB surface
    // So if post CSC is needed, it will always be RGB to YUV conversion
    if (stageEnable.PostCSCEnable)
    {
        if (targetCSpace == CSpace_BT601)
        {
            currentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT601;
        }
        else if (targetCSpace == CSpace_BT709)
        {
            currentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT709;
        }
        else if (targetCSpace == CSpace_BT709_FullRange)
        {
            // CSC for target BT709_FULLRANGE is only exposed to Vebox Preprocessed HDR cases.
            currentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT709_FULLRANGE;
        }
        else if (targetCSpace == CSpace_BT2020 ||
                 targetCSpace == CSpace_BT2020_FullRange)
        {
            currentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT2020;
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    if (param.LUTMode != currentLUTMode ||
        param.EOTFGamma != currentEOTF ||
        param.OETFGamma != currentOETF ||
        param.CCM != currentCCM ||
        param.CCMExt1 != currentCCMExt1 ||
        param.CCMExt2 != currentCCMExt2 ||
        param.HdrMode != currentHdrMode ||
        param.PriorCSC != currentPriorCSC ||
        param.PostCSC != currentPostCSC)
    {
        needUpdate = true;
    }

    if (memcmp(&param.hdrParams, &m_lastFrameSourceParams[index], sizeof(HDR_PARAMS)))
    {
        needUpdate                     = true;
        m_lastFrameSourceParams[index] = param.hdrParams;
    }

    param.LUTMode   = currentLUTMode;
    param.EOTFGamma = currentEOTF;
    param.OETFGamma = currentOETF;
    param.CCM       = currentCCM;
    param.CCMExt1   = currentCCMExt1;
    param.CCMExt2   = currentCCMExt2;
    param.HdrMode   = currentHdrMode;
    param.PriorCSC  = currentPriorCSC;
    param.PostCSC   = currentPostCSC;
    param.enabled   = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::InitLayerOETF1DLUT(VpAllocator &allocator, HDR_PARAMS &targetHdrParam, HDRLITE_LAYER_PARAM &layerParam, PVP_SURFACE oetfSurface)
{
    VP_PUBLIC_CHK_NULL_RETURN(oetfSurface);
    VP_PUBLIC_CHK_NULL_RETURN(oetfSurface->osSurface);

    MOS_LOCK_PARAMS lockFlags = {};
    MOS_ZeroMemory(&lockFlags, sizeof(lockFlags));
    lockFlags.WriteOnly = 1;
    // Lock the surface for writing
    uint8_t *dstOetfLut = (uint8_t *)allocator.Lock(&(oetfSurface->osSurface->OsResource), &lockFlags);
    auto     autoUnlock = std::shared_ptr<void>(nullptr, [&](void *) {
        if (allocator.UnLock(&oetfSurface->osSurface->OsResource) != MOS_STATUS_SUCCESS)
        {
            VP_RENDER_ASSERTMESSAGE("Unlock Fail Error");
        }
    });
    VP_PUBLIC_CHK_NULL_RETURN(dstOetfLut);

    uint16_t *srcOetfLut = nullptr;

    // Hdr kernel require 0 to 1 floating point color value
    // To transfer the value of 16bit integer OETF table to 0 to 1 floating point
    // We need to divide the table with 2^16 - 1
    if (targetHdrParam.EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR ||
        targetHdrParam.EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR)
    {
        if (layerParam.OETFGamma == VPHAL_GAMMA_SRGB)
        {
            srcOetfLut = (uint16_t *)g_Hdr_ColorCorrect_OETF_sRGB_FP16;
        }
        else
        {
            srcOetfLut = (uint16_t *)g_Hdr_ColorCorrect_OETF_BT709_FP16;
        }

        for (uint32_t i = 0; i < oetfSurface->osSurface->dwHeight; ++i, dstOetfLut += oetfSurface->osSurface->dwPitch, srcOetfLut += oetfSurface->osSurface->dwWidth)
        {
            MOS_SecureMemcpy(dstOetfLut, sizeof(uint16_t) * oetfSurface->osSurface->dwWidth, srcOetfLut, sizeof(uint16_t) * oetfSurface->osSurface->dwWidth);
        }
    }
    else if (targetHdrParam.EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
    {
        uint16_t oetfSmpteSt2084[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER];
        if (layerParam.HdrMode == VPHAL_HDR_MODE_INVERSE_TONE_MAPPING)
        {
            static auto HdrOETF2084 = [](float c) -> float {
                static const double C1 = 0.8359375;
                static const double C2 = 18.8515625;
                static const double C3 = 18.6875;
                static const double M1 = 0.1593017578125;
                static const double M2 = 78.84375;

                double tmp         = c;
                double numerator   = pow(tmp, M1);
                double denominator = numerator;

                denominator = 1.0 + C3 * denominator;
                numerator   = C1 + C2 * numerator;
                numerator   = numerator / denominator;

                return (float)pow(numerator, M2);
            };

            const float fStretchFactor = 0.01f;
            VP_PUBLIC_CHK_STATUS_RETURN(Generate2SegmentsOETFLUT(fStretchFactor, HdrOETF2084, oetfSmpteSt2084));
            srcOetfLut = oetfSmpteSt2084;
        }
        else  // params->HdrMode[iIndex] == VPHAL_HDR_MODE_H2H
        {
            srcOetfLut = (uint16_t *)g_Hdr_ColorCorrect_OETF_SMPTE_ST2084_3Segs_FP16;
        }

        for (uint32_t i = 0; i < oetfSurface->osSurface->dwHeight; ++i, dstOetfLut += oetfSurface->osSurface->dwPitch, srcOetfLut += oetfSurface->osSurface->dwWidth)
        {
            MOS_SecureMemcpy(dstOetfLut, sizeof(uint16_t) * oetfSurface->osSurface->dwWidth, srcOetfLut, sizeof(uint16_t) * oetfSurface->osSurface->dwWidth);
        }
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::InitLayerCri3DLut(VpAllocator &allocator, HDR_PARAMS &targetHdrParam, HDRLITE_LAYER_PARAM &layerParam, PVP_SURFACE inputSurface, PVP_SURFACE cri3DLutSurface)
{
    VP_PUBLIC_CHK_NULL_RETURN(cri3DLutSurface);
    VP_PUBLIC_CHK_NULL_RETURN(cri3DLutSurface->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurface->osSurface);

    MOS_LOCK_PARAMS lockFlags = {};
    MOS_ZeroMemory(&lockFlags, sizeof(lockFlags));
    lockFlags.WriteOnly = 1;
    // Lock the surface for writing
    uint8_t *baseCri3DLut = (uint8_t *)allocator.Lock(&(cri3DLutSurface->osSurface->OsResource), &lockFlags);
    auto     autoUnlock   = std::shared_ptr<void>(nullptr, [&](void *) {
        if (allocator.UnLock(&cri3DLutSurface->osSurface->OsResource) != MOS_STATUS_SUCCESS)
        {
            VP_RENDER_ASSERTMESSAGE("Unlock Fail Error");
        }
    });
    VP_PUBLIC_CHK_NULL_RETURN(baseCri3DLut);

    if (cri3DLutSurface->osSurface->Format == Format_A16B16G16R16)
    {
        uint8_t bytePerPixel = 8;

        for (uint32_t i = 0; i < VPHAL_HDR_CRI_3DLUT_SIZE; ++i)
        {
            for (uint32_t j = 0; j < VPHAL_HDR_CRI_3DLUT_SIZE; ++j)
            {
                for (uint32_t k = 0; k < VPHAL_HDR_CRI_3DLUT_SIZE; ++k)
                {
                    uint16_t *dst3DLut = (uint16_t *)(baseCri3DLut +
                                                      i * VPHAL_HDR_CRI_3DLUT_SIZE * cri3DLutSurface->osSurface->dwPitch +
                                                      j * cri3DLutSurface->osSurface->dwPitch +
                                                      k * bytePerPixel);

                    uint16_t outputX = 0;
                    uint16_t outputY = 0;
                    uint16_t outputZ = 0;

                    VP_PUBLIC_CHK_STATUS_RETURN(GenerateColorTransfer3dLut(
                        layerParam,
                        targetHdrParam,
                        inputSurface->osSurface->Format,
                        (float)k / (float)(VPHAL_HDR_CRI_3DLUT_SIZE - 1),
                        (float)j / (float)(VPHAL_HDR_CRI_3DLUT_SIZE - 1),
                        (float)i / (float)(VPHAL_HDR_CRI_3DLUT_SIZE - 1),
                        outputX,
                        outputY,
                        outputZ));

                    *dst3DLut++ = outputX;
                    *dst3DLut++ = outputY;
                    *dst3DLut++ = outputZ;
                }
            }
        }
    }
    else if (cri3DLutSurface->osSurface->Format == Format_R10G10B10A2)
    {
        uint8_t bytePerPixel = 4;

        for (uint32_t i = 0; i < VPHAL_HDR_CRI_3DLUT_SIZE; i++)
        {
            for (uint32_t j = 0; j < VPHAL_HDR_CRI_3DLUT_SIZE; j++)
            {
                for (uint32_t k = 0; k < VPHAL_HDR_CRI_3DLUT_SIZE; k++)
                {
                    uint32_t *dst3dLut = (uint32_t *)(baseCri3DLut +
                                                      i * VPHAL_HDR_CRI_3DLUT_SIZE * cri3DLutSurface->osSurface->dwPitch +
                                                      j * cri3DLutSurface->osSurface->dwPitch +
                                                      k * bytePerPixel);

                    uint16_t outputX = 0;
                    uint16_t outputY = 0;
                    uint16_t outputZ = 0;

                    VP_PUBLIC_CHK_STATUS_RETURN(GenerateColorTransfer3dLut(
                        layerParam,
                        targetHdrParam,
                        inputSurface->osSurface->Format,
                        (float)k / (float)(VPHAL_HDR_CRI_3DLUT_SIZE - 1),
                        (float)j / (float)(VPHAL_HDR_CRI_3DLUT_SIZE - 1),
                        (float)i / (float)(VPHAL_HDR_CRI_3DLUT_SIZE - 1),
                        outputX,
                        outputY,
                        outputZ));

                    *dst3dLut = (uint32_t)outputX +
                                ((uint32_t)outputY << 10) +
                                ((uint32_t)outputZ << 20);
                }
            }
        }
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::InitCoeff(VpAllocator &allocator, HDRLITE_HAL_PARAM &param, VP_SURFACE_GROUP &surfGroup)
{
    auto coeffHandle = surfGroup.find(SurfaceTypeHdrCoeff);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(coeffHandle, &surfGroup)
    VP_RENDER_CHK_NULL_RETURN(coeffHandle->second);
    VP_RENDER_CHK_NULL_RETURN(coeffHandle->second->osSurface);

    MOS_LOCK_PARAMS lockFlags = {};
    MOS_ZeroMemory(&lockFlags, sizeof(lockFlags));
    lockFlags.WriteOnly = 1;
    // Lock the surface for writing
    float *coeffBase  = (float *)allocator.Lock(&(coeffHandle->second->osSurface->OsResource), &lockFlags);
    auto   autoUnlock = std::shared_ptr<void>(nullptr, [&](void *) {
        if (allocator.UnLock(&coeffHandle->second->osSurface->OsResource) != MOS_STATUS_SUCCESS)
        {
            VP_RENDER_ASSERTMESSAGE("Unlock Fail Error");
        }
    });
    VP_PUBLIC_CHK_NULL_RETURN(coeffBase);

    uint64_t pitch      = coeffHandle->second->osSurface->dwPitch;
    uint64_t line       = pitch / sizeof(float);
    float   *layerEnd   = coeffBase + VPHAL_HDR_COEF_LINES_PER_LAYER_BASIC * line * 8;
    float   *ccmExtBase = layerEnd + 2 * line;  // Skip the Dst CSC area

    // Coef[64][7] is to normalize the fp16 input value
    *(layerEnd + 7) = 1.0f / 125.0f;

    for (uint32_t i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; ++i)
    {
        float *curLayerBase       = coeffBase + VPHAL_HDR_COEF_LINES_PER_LAYER_BASIC * line * i;
        float *curLayerCCMExtBase = ccmExtBase + VPHAL_HDR_COEF_LINES_PER_LAYER_EXT * line * i;

        if (!param.inputLayerParam[i].enabled)
        {
            continue;
        }

        VP_PUBLIC_CHK_STATUS_RETURN(InitLayerCoeffBase(param.targetHdrParams, param.inputLayerParam[i], line, param.uiMaxDisplayLum, curLayerBase));
        VP_PUBLIC_CHK_STATUS_RETURN(InitLayerCoeffCCMExt(param.targetHdrParams, param.inputLayerParam[i], line, curLayerCCMExtBase));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::InitLayerCoeffBase(HDR_PARAMS &targetHdrParam, HDRLITE_LAYER_PARAM &layerParam, uint64_t line, uint32_t maxDisplayLum, float *dst)
{
    HDRStageEnables &stageEnable  = layerParam.stageEnable;
    uint32_t        &eotfType     = *(uint32_t *)(dst + VPHAL_HDR_COEF_EOTF_OFFSET);
    float           *eotfCoeffDst = dst + line + VPHAL_HDR_COEF_EOTF_OFFSET;
    uint32_t        &oetfType     = *(uint32_t *)(dst + VPHAL_HDR_COEF_EOTF_OFFSET + 1);
    float           *oetfCoeffDst = dst + line + VPHAL_HDR_COEF_EOTF_OFFSET + 1;

    auto WriteMatrix = [](const float matrix[], uint64_t line, float *&dst) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(matrix);
        VP_PUBLIC_CHK_NULL_RETURN(dst);
        *dst++ = matrix[0];
        *dst++ = matrix[1];
        *dst++ = matrix[2];
        *dst++ = matrix[3];
        *dst++ = matrix[4];
        *dst++ = matrix[5];
        dst += line - 6;
        *dst++ = matrix[6];
        *dst++ = matrix[7];
        *dst++ = matrix[8];
        *dst++ = matrix[9];
        *dst++ = matrix[10];
        *dst++ = matrix[11];
        dst += line - 6;
        return MOS_STATUS_SUCCESS;
    };

    auto ConvertCCMMatrix = [](const std::array<float, 12> &transferMatrix, float *outMatrix) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(outMatrix);
        // multiplication of + onwards
        outMatrix[0]  = transferMatrix[1];
        outMatrix[1]  = transferMatrix[2];
        outMatrix[2]  = transferMatrix[0];
        outMatrix[4]  = transferMatrix[5];
        outMatrix[5]  = transferMatrix[6];
        outMatrix[6]  = transferMatrix[4];
        outMatrix[8]  = transferMatrix[9];
        outMatrix[9]  = transferMatrix[10];
        outMatrix[10] = transferMatrix[8];

        outMatrix[3]  = transferMatrix[11];
        outMatrix[7]  = transferMatrix[3];
        outMatrix[11] = transferMatrix[7];
        return MOS_STATUS_SUCCESS;
    };

    auto SetTF = [](float c1, float c2, float c3, float c4, float c5, uint64_t line, float *out) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(out);
        *out = c1;
        out += line;
        *out = c2;
        out += line;
        *out = c3;
        out += line;
        *out = c4;
        out += line;
        *out = c5;
        return MOS_STATUS_SUCCESS;
    };

    // EOTF/CCM/Tone Mapping/OETF require RGB input
    // So if prior CSC is needed, it will always be YUV to RGB conversion
    if (stageEnable.PriorCSCEnable)
    {
        float priorCscMatrix[12] = {};
        VP_PUBLIC_CHK_STATUS_RETURN(CalculateCscMatrix(layerParam.PriorCSC, false, priorCscMatrix));
        for (uint32_t i = 0; i < 12; ++i)
        {
            // To keep the precision for kernel calculation
            priorCscMatrix[i] = static_cast<float>(ConvertRegister2DoubleFormat(ConvertDouble2RegisterForamt(static_cast<double>(priorCscMatrix[i]))));
        }
        VP_PUBLIC_CHK_STATUS_RETURN(WriteMatrix(priorCscMatrix, line, dst));
    }
    else
    {
        dst += line * 2;
    }

    if (stageEnable.CCMEnable)
    {
        std::array<float, 12> matrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        // BT709 to BT2020 CCM
        if (layerParam.CCM == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
        {
            matrix = {0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f};
        }
        // BT2020 to BT709 CCM
        else if (layerParam.CCM == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
        {
            matrix = {1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f};
        }

        float ccmMatrix[12] = {};
        VP_PUBLIC_CHK_STATUS_RETURN(ConvertCCMMatrix(matrix, ccmMatrix));
        for (uint32_t i = 0; i < 12; ++i)
        {
            // To keep the precision for kernel calculation
            ccmMatrix[i] = static_cast<float>(ConvertRegister2DoubleFormat(ConvertDouble2RegisterForamt(static_cast<double>(ccmMatrix[i]))));
        }
        VP_PUBLIC_CHK_STATUS_RETURN(WriteMatrix(ccmMatrix, line, dst));
    }
    else
    {
        dst += line * 2;
    }

    // OETF will output RGB surface
    // So if post CSC is needed, it will always be RGB to YUV conversion
    if (stageEnable.PostCSCEnable)
    {
        float postCscMatrix[12] = {};
        VP_PUBLIC_CHK_STATUS_RETURN(CalculateCscMatrix(layerParam.PostCSC, false, postCscMatrix));
        for (uint32_t i = 0; i < 12; ++i)
        {
            // To keep the precision for kernel calculation
            postCscMatrix[i] = static_cast<float>(ConvertRegister2DoubleFormat(ConvertDouble2RegisterForamt(static_cast<double>(postCscMatrix[i]))));
        }
        VP_PUBLIC_CHK_STATUS_RETURN(WriteMatrix(postCscMatrix, line, dst));
    }
    else
    {
        dst += line * 2;
    }

    if (stageEnable.EOTFEnable)
    {
        if (layerParam.EOTFGamma == VPHAL_GAMMA_TRADITIONAL_GAMMA)
        {
            eotfType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
            VP_PUBLIC_CHK_STATUS_RETURN(SetTF(
                VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA,
                VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA,
                VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA,
                VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA,
                VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA,
                line,
                eotfCoeffDst));
        }
        else if (layerParam.EOTFGamma == VPHAL_GAMMA_SMPTE_ST2084)
        {
            eotfType = VPHAL_HDR_KERNEL_SMPTE_ST2084;
            VP_PUBLIC_CHK_STATUS_RETURN(SetTF(
                VPHAL_HDR_EOTF_COEFF1_SMPTE_ST2084,
                VPHAL_HDR_EOTF_COEFF2_SMPTE_ST2084,
                VPHAL_HDR_EOTF_COEFF3_SMPTE_ST2084,
                VPHAL_HDR_EOTF_COEFF4_SMPTE_ST2084,
                VPHAL_HDR_EOTF_COEFF5_SMPTE_ST2084,
                line,
                eotfCoeffDst));
        }
        else if (layerParam.EOTFGamma == VPHAL_GAMMA_BT1886)
        {
            eotfType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
            VP_PUBLIC_CHK_STATUS_RETURN(SetTF(
                VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_BT1886,
                VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_BT1886,
                VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_BT1886,
                VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_BT1886,
                VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_BT1886,
                line,
                eotfCoeffDst));
        }
        else if (layerParam.EOTFGamma == VPHAL_GAMMA_SRGB)
        {
            eotfType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
            VP_PUBLIC_CHK_STATUS_RETURN(SetTF(
                VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_SRGB,
                VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_SRGB,
                VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_SRGB,
                VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_SRGB,
                VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_SRGB,
                line,
                eotfCoeffDst));
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    if (stageEnable.OETFEnable)
    {
        if (layerParam.OETFGamma == VPHAL_GAMMA_TRADITIONAL_GAMMA)
        {
            oetfType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
            VP_PUBLIC_CHK_STATUS_RETURN(SetTF(
                VPHAL_HDR_OETF_COEFF1_TRADITIONNAL_GAMMA,
                VPHAL_HDR_OETF_COEFF2_TRADITIONNAL_GAMMA,
                VPHAL_HDR_OETF_COEFF3_TRADITIONNAL_GAMMA,
                VPHAL_HDR_OETF_COEFF4_TRADITIONNAL_GAMMA,
                VPHAL_HDR_OETF_COEFF5_TRADITIONNAL_GAMMA,
                line,
                oetfCoeffDst));
        }
        else if (layerParam.OETFGamma == VPHAL_GAMMA_SRGB)
        {
            oetfType = VPHAL_HDR_KERNEL_EOTF_TRADITIONAL_GAMMA;
            VP_PUBLIC_CHK_STATUS_RETURN(SetTF(
                VPHAL_HDR_OETF_COEFF1_TRADITIONNAL_GAMMA_SRGB,
                VPHAL_HDR_OETF_COEFF2_TRADITIONNAL_GAMMA_SRGB,
                VPHAL_HDR_OETF_COEFF3_TRADITIONNAL_GAMMA_SRGB,
                VPHAL_HDR_OETF_COEFF4_TRADITIONNAL_GAMMA_SRGB,
                VPHAL_HDR_OETF_COEFF5_TRADITIONNAL_GAMMA_SRGB,
                line,
                oetfCoeffDst));
        }
        else if (layerParam.OETFGamma == VPHAL_GAMMA_SMPTE_ST2084)
        {
            oetfType = VPHAL_HDR_KERNEL_SMPTE_ST2084;
            VP_PUBLIC_CHK_STATUS_RETURN(SetTF(
                VPHAL_HDR_OETF_COEFF1_SMPTE_ST2084,
                VPHAL_HDR_OETF_COEFF2_SMPTE_ST2084,
                VPHAL_HDR_OETF_COEFF3_SMPTE_ST2084,
                VPHAL_HDR_OETF_COEFF4_SMPTE_ST2084,
                VPHAL_HDR_OETF_COEFF5_SMPTE_ST2084,
                line,
                oetfCoeffDst));
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    float    *pivotPoint     = dst;
    uint16_t *slopeIntercept = (uint16_t *)(dst + line);
    float    &pwlfStretch    = *(dst + 5);
    uint32_t &tmType         = *(uint32_t *)(dst + 5);
    float    &coeffR         = *(dst + 6);
    float    &coeffG         = *(dst + 7);
    float    &coeffB         = *(dst + 6 + line);
    uint32_t &oetfNeqType    = *(uint32_t *)(dst + line + 7);

    if (layerParam.HdrMode == VPHAL_HDR_MODE_TONE_MAPPING)
    {
        tmType      = 1;                  // TMtype
        oetfNeqType = 0 | (10000 << 16);  // OETFNEQ
        coeffR      = 0.25f;
        coeffG      = 0.625f;
        coeffB      = 0.125f;

        float pivot0X = 0.0f, pivot1X = 0.03125f, pivot2X = 0.09375f, pivot3X = 0.125f, pivot4X = 0.21875f, pivot5X = 0.40625f;
        float pivot0Y = 0.0f, pivot1Y = 0.7f, pivot2Y = 0.9f, pivot3Y = 0.95f, pivot4Y = 0.99f, pivot5Y = 1.0f;
        float sf1, sf2, sf3, sf4, sf5;
        float in1f, in2f, in3f, in4f, in5f;

        // Calculate Gradient and Intercepts
        sf1     = (pivot1X - pivot0X) > 0.0f ? (float)(pivot1Y - pivot0Y) / (pivot1X - pivot0X) : 0.0f;
        pivot1Y = sf1 * (pivot1X - pivot0X) + pivot0Y;

        sf2     = (pivot2X - pivot1X) > 0.0f ? (float)(pivot2Y - pivot1Y) / (pivot2X - pivot1X) : 0.0f;
        pivot2Y = sf2 * (pivot2X - pivot1X) + pivot1Y;

        sf3     = (pivot3X - pivot2X) > 0.0f ? (float)(pivot3Y - pivot2Y) / (pivot3X - pivot2X) : 0.0f;
        pivot3Y = sf3 * (pivot3X - pivot2X) + pivot2Y;

        sf4     = (pivot4X - pivot3X) > 0.0f ? (float)(pivot4Y - pivot3Y) / (pivot4X - pivot3X) : 0.0f;
        pivot4Y = sf4 * (pivot4X - pivot3X) + pivot3Y;

        sf5      = (pivot5X - pivot4X) > 0.0f ? (float)(pivot5Y - pivot4Y) / (pivot5X - pivot4X) : 0.0f;
        pivot5Y = sf5 * (pivot5X - pivot4X) + pivot4Y;

        // Calculating Intercepts
        in1f = pivot0Y;
        in2f = pivot1Y - (sf2 * pivot1X);
        in3f = pivot2Y - (sf3 * pivot2X);
        in4f = pivot3Y - (sf4 * pivot3X);
        in5f = pivot4Y - (sf5 * pivot4X);

        // Pivot Point
        *pivotPoint++ = pivot1X;
        *pivotPoint++ = pivot2X;
        *pivotPoint++ = pivot3X;
        *pivotPoint++ = pivot4X;
        *pivotPoint++ = pivot5X;

        // Slope and Intercept
        *slopeIntercept++ = VpHal_FloatToHalfFloat(sf1);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(in1f);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(sf2);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(in2f);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(sf3);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(in3f);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(sf4);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(in4f);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(sf5);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(in5f);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(0.0f);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(pivot5Y);
    }
    else if (layerParam.HdrMode == VPHAL_HDR_MODE_INVERSE_TONE_MAPPING)
    {
        pwlfStretch = 0.01f;                      // Stretch
        oetfNeqType = 1 | ((uint32_t)100 << 16);  // OETFNEQ
        coeffR      = 0.0f;
        coeffG      = 0.0f;
        coeffB      = 0.0f;

        // Pivot Point
        *pivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X1;
        *pivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X2;
        *pivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X3;
        *pivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X4;
        *pivotPoint++ = VPHAL_HDR_INVERSE_TONE_MAPPING_PIVOT_POINT_X5;

        // Slope and Intercept
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE0);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT0);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE1);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT1);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE2);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT2);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE3);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT3);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE4);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT4);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_SLOPE5);
        *slopeIntercept++ = VpHal_FloatToHalfFloat(VPHAL_HDR_INVERSE_TONE_MAPPING_INTERCEPT5);
    }
    else if (layerParam.HdrMode == VPHAL_HDR_MODE_H2H ||
             layerParam.HdrMode == VPHAL_HDR_MODE_H2H_AUTO_MODE)
    {
        tmType      = 1;                                                  // TMtype
        oetfNeqType = 2 | (((uint32_t)maxDisplayLum) << 16);  // OETFNEQ
        coeffR      = 0.25f;
        coeffG      = 0.625f;
        coeffB      = 0.125f;

        VP_PUBLIC_CHK_STATUS_RETURN(GenerateH2HPWLFCoeff(layerParam.hdrParams, targetHdrParam, pivotPoint, slopeIntercept));
    }
    else
    {
        *pivotPoint = 0.0f;
        tmType      = 0;  // TMtype
        oetfNeqType = 0;  // OETFNEQ
    }
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::InitLayerCoeffCCMExt(HDR_PARAMS &targetHdrParam, HDRLITE_LAYER_PARAM &layerParam, uint64_t line, float *dst)
{
    auto WriteMatrix = [](const float matrix[], uint64_t line, float *&dst) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(matrix);
        VP_PUBLIC_CHK_NULL_RETURN(dst);
        *dst++ = matrix[0];
        *dst++ = matrix[1];
        *dst++ = matrix[2];
        *dst++ = matrix[3];
        *dst++ = matrix[4];
        *dst++ = matrix[5];
        dst += line - 6;
        *dst++ = matrix[6];
        *dst++ = matrix[7];
        *dst++ = matrix[8];
        *dst++ = matrix[9];
        *dst++ = matrix[10];
        *dst++ = matrix[11];
        dst += line - 6;
        return MOS_STATUS_SUCCESS;
    };

    auto ConvertCCMMatrix = [](const std::array<float, 12> &transferMatrix, float *outMatrix) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(outMatrix);
        // multiplication of + onwards
        outMatrix[0]  = transferMatrix[1];
        outMatrix[1]  = transferMatrix[2];
        outMatrix[2]  = transferMatrix[0];
        outMatrix[4]  = transferMatrix[5];
        outMatrix[5]  = transferMatrix[6];
        outMatrix[6]  = transferMatrix[4];
        outMatrix[8]  = transferMatrix[9];
        outMatrix[9]  = transferMatrix[10];
        outMatrix[10] = transferMatrix[8];

        outMatrix[3]  = transferMatrix[11];
        outMatrix[7]  = transferMatrix[3];
        outMatrix[11] = transferMatrix[7];
        return MOS_STATUS_SUCCESS;
    };

    HDRStageEnables &stageEnable     = layerParam.stageEnable;
    uint32_t        &ccm1Enable      = *(uint32_t *)(dst + VPHAL_HDR_COEF_CCMEXT_OFFSET);
    uint32_t        &ccm1ClampEnable = *(uint32_t *)(dst + VPHAL_HDR_COEF_CLAMP_OFFSET);
    uint32_t        &ccm2Enable      = *(uint32_t *)(dst + line * 2 + VPHAL_HDR_COEF_CCMEXT_OFFSET);
    uint32_t        &ccm2ClampEnable = *(uint32_t *)(dst + line * 2 + VPHAL_HDR_COEF_CLAMP_OFFSET);

    ccm1Enable      = stageEnable.CCMExt1Enable;
    ccm1ClampEnable = stageEnable.GamutClamp1Enable;
    ccm2Enable      = stageEnable.CCMExt2Enable;
    ccm2ClampEnable = stageEnable.GamutClamp2Enable;

    if (stageEnable.CCMExt1Enable)
    {
        std::array<float, 12> matrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        // BT709 to BT2020 CCM
        if (layerParam.CCMExt1 == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
        {
            matrix = {0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f};
        }
        // BT2020 to BT709 CCM
        else if (layerParam.CCMExt1 == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
        {
            matrix = {1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f};
        }
        else if (layerParam.CCMExt1 == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                 layerParam.CCMExt1 == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                 layerParam.CCMExt1 == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(CalculateCCMWithMonitorGamut(layerParam.CCMExt1, targetHdrParam, matrix));
        }

        float ccmMatrix[12] = {};
        VP_PUBLIC_CHK_STATUS_RETURN(ConvertCCMMatrix(matrix, ccmMatrix));
        for (uint32_t i = 0; i < 12; ++i)
        {
            // To keep the precision for kernel calculation
            ccmMatrix[i] = static_cast<float>(ConvertRegister2DoubleFormat(ConvertDouble2RegisterForamt(static_cast<double>(ccmMatrix[i]))));
        }
        VP_PUBLIC_CHK_STATUS_RETURN(WriteMatrix(ccmMatrix, line, dst));
    }
    else
    {
        dst += line * 2;
    }

    if (stageEnable.CCMExt2Enable)
    {
        std::array<float, 12> matrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        // BT709 to BT2020 CCM
        if (layerParam.CCMExt2 == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
        {
            matrix = {0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f};
        }
        // BT2020 to BT709 CCM
        else if (layerParam.CCMExt2 == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
        {
            matrix = {1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f};
        }
        else if (layerParam.CCMExt2 == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                 layerParam.CCMExt2 == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                 layerParam.CCMExt2 == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(CalculateCCMWithMonitorGamut(layerParam.CCMExt2, targetHdrParam, matrix));
        }

        float ccmMatrix[12] = {};
        VP_PUBLIC_CHK_STATUS_RETURN(ConvertCCMMatrix(matrix, ccmMatrix));
        for (uint32_t i = 0; i < 12; ++i)
        {
            // To keep the precision for kernel calculation
            ccmMatrix[i] = static_cast<float>(ConvertRegister2DoubleFormat(ConvertDouble2RegisterForamt(static_cast<double>(ccmMatrix[i]))));
        }
        VP_PUBLIC_CHK_STATUS_RETURN(WriteMatrix(ccmMatrix, line, dst));
    }
    else
    {
        dst += line * 2;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::GenerateH2HPWLFCoeff(HDR_PARAMS &srcHdrParam, HDR_PARAMS &targetHdrParam, float *pivotPoint, uint16_t *slopeIntercept)
{
    VP_PUBLIC_CHK_NULL_RETURN(pivotPoint);
    VP_PUBLIC_CHK_NULL_RETURN(slopeIntercept);

    const float minLumDisp = targetHdrParam.min_display_mastering_luminance / 10000.0f / 10000.0f;
    const float maxLumDisp = targetHdrParam.max_display_mastering_luminance / 10000.0f;
    const bool  align33Lut = true;
    const int   lutEntries = 32;
    const float lutStep    = 1.0f / lutEntries;
    const float split      = 0.7f;
    
    float pivot0X = 0;
    float pivot1X = 0.0313f;
    float pivot2X = 0;
    float pivot3X = 0;
    float pivot4X = 0;
    float pivot5X = srcHdrParam.MaxCLL / 10000.0f;

    float pivot0Y = 0;  // Force Y0 to zero to workaround a green bar issue.
    float pivot1Y = 0.0313f;
    float pivot2Y = 0;
    float pivot3Y = 0;
    float pivot4Y = 0;
    float pivot5Y = maxLumDisp;

    float sf1 = 0;
    float sf2 = 0;
    float sf3 = 0;
    float sf4 = 0;
    float sf5 = 0;

    float in1f = 0;
    float in2f = 0;
    float in3f = 0;
    float in4f = 0;
    float in5f = 0;

    if (targetHdrParam.max_display_mastering_luminance >= srcHdrParam.MaxCLL)
    {
        pivot2X = pivot3X = pivot4X = pivot5X = pivot2Y = pivot3Y = pivot4Y = maxLumDisp;
    }
    else
    {
        if (align33Lut)
        {
            pivot5X = ceil(pivot5X / lutStep) * lutStep;
        }

        pivot2X = pivot1X + (pivot5X - pivot1X) / 5.0f;
        pivot3X = pivot1X + (pivot5X - pivot1X) * 2.0f / 5.0f;
        pivot4X = pivot1X + (pivot5X - pivot1X) * 3.0f / 5.0f;

        if (align33Lut)
        {
            pivot2X = floor(pivot2X / lutStep) * lutStep;
            pivot3X = floor(pivot3X / lutStep) * lutStep;
            pivot4X = floor(pivot4X / lutStep) * lutStep;
        }

        pivot4Y = pivot5Y * 0.95f;
        if (pivot4Y > pivot4X)
        {
            pivot4Y = pivot4X;
        }

        pivot2Y = pivot1Y + (pivot4Y - pivot1Y) * split;
        if (pivot2Y > pivot2X)
        {
            pivot2Y = pivot2X;
        }

        pivot3Y = pivot2Y + (pivot4Y - pivot2Y) * split;
        if (pivot3Y > pivot3X)
        {
            pivot3Y = pivot3X;
        }
    }

    // Calculate Gradient and Intercepts
    sf1     = (pivot1X - pivot0X) > 0.0f ? (float)(pivot1Y - pivot0Y) / (pivot1X - pivot0X) : 0.0f;
    pivot1Y = sf1 * (pivot1X - pivot0X) + pivot0Y;

    sf2     = (pivot2X - pivot1X) > 0.0f ? (float)(pivot2Y - pivot1Y) / (pivot2X - pivot1X) : 0.0f;
    pivot2Y = sf2 * (pivot2X - pivot1X) + pivot1Y;

    sf3     = (pivot3X - pivot2X) > 0.0f ? (float)(pivot3Y - pivot2Y) / (pivot3X - pivot2X) : 0.0f;
    pivot3Y = sf3 * (pivot3X - pivot2X) + pivot2Y;

    sf4     = (pivot4X - pivot3X) > 0.0f ? (float)(pivot4Y - pivot3Y) / (pivot4X - pivot3X) : 0.0f;
    pivot4Y = sf4 * (pivot4X - pivot3X) + pivot3Y;

    sf5     = (pivot5X - pivot4X) > 0.0f ? (float)(pivot5Y - pivot4Y) / (pivot5X - pivot4X) : 0.0f;
    pivot5Y = sf5 * (pivot5X - pivot4X) + pivot4Y;

    // Calculating Intercepts
    in1f = pivot0Y;
    in2f = pivot1Y - (sf2 * pivot1X);
    in3f = pivot2Y - (sf3 * pivot2X);
    in4f = pivot3Y - (sf4 * pivot3X);
    in5f = pivot4Y - (sf5 * pivot4X);

    pivotPoint[0] = pivot1X;
    pivotPoint[1] = pivot2X;
    pivotPoint[2] = pivot3X;
    pivotPoint[3] = pivot4X;
    pivotPoint[4] = pivot5X;

    slopeIntercept[0]  = VpHal_FloatToHalfFloat(sf1);
    slopeIntercept[1]  = VpHal_FloatToHalfFloat(in1f);
    slopeIntercept[2]  = VpHal_FloatToHalfFloat(sf2);
    slopeIntercept[3]  = VpHal_FloatToHalfFloat(in2f);
    slopeIntercept[4]  = VpHal_FloatToHalfFloat(sf3);
    slopeIntercept[5]  = VpHal_FloatToHalfFloat(in3f);
    slopeIntercept[6]  = VpHal_FloatToHalfFloat(sf4);
    slopeIntercept[7]  = VpHal_FloatToHalfFloat(in4f);
    slopeIntercept[8]  = VpHal_FloatToHalfFloat(sf5);
    slopeIntercept[9]  = VpHal_FloatToHalfFloat(in5f);
    slopeIntercept[10] = VpHal_FloatToHalfFloat(0.0f);  // Saturation
    slopeIntercept[11] = VpHal_FloatToHalfFloat(pivot5Y);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::GenerateColorTransfer3dLut(HDRLITE_LAYER_PARAM &param, HDR_PARAMS &targetHdrParam, MOS_FORMAT inputFormat, float fInputX, float fInputY, float fInputZ, uint16_t &outputX, uint16_t &outputY, uint16_t &outputZ)
{
    HDRStageEnables &stageEnable = param.stageEnable;
    double           m1          = 0.1593017578125;  // SMPTE ST2084 EOTF parameters
    double           m2          = 78.84375;         // SMPTE ST2084 EOTF parameters
    double           c2          = 18.8515625;       // SMPTE ST2084 EOTF parameters
    double           c3          = 18.6875;          // SMPTE ST2084 EOTF parameters
    double           c1          = c3 - c2 + 1;      // SMPTE ST2084 EOTF parameters

    double resultX = (double)fInputX;
    double resultY = (double)fInputY;
    double resultZ = (double)fInputZ;

    // EOTF/CCM/Tone Mapping/OETF require RGB input
    // So if prior CSC is needed, it will always be YUV to RGB conversion
    if (stageEnable.PriorCSCEnable)
    {
        float priorCscMatrix[12] = {};
        VP_PUBLIC_CHK_STATUS_RETURN(CalculateCscMatrix(param.PriorCSC, true, priorCscMatrix));

        double tempX = resultX;
        double tempY = resultY;
        double tempZ = resultZ;

        if (inputFormat == Format_AYUV)
        {
            resultX = priorCscMatrix[0] * resultY + priorCscMatrix[1] * resultZ + priorCscMatrix[2] * tempX + priorCscMatrix[3];
            resultY = priorCscMatrix[4] * resultY + priorCscMatrix[5] * resultZ + priorCscMatrix[6] * tempX + priorCscMatrix[7];
            resultZ = priorCscMatrix[8] * resultY + priorCscMatrix[9] * resultZ + priorCscMatrix[10] * tempX + priorCscMatrix[11];
        }
        else
        {
            resultX = priorCscMatrix[0] * resultZ + priorCscMatrix[1] * resultY + priorCscMatrix[2] * tempX + priorCscMatrix[3];
            resultY = priorCscMatrix[4] * resultZ + priorCscMatrix[5] * resultY + priorCscMatrix[6] * tempX + priorCscMatrix[7];
            resultZ = priorCscMatrix[8] * resultZ + priorCscMatrix[9] * resultY + priorCscMatrix[10] * tempX + priorCscMatrix[11];
        }

        MOS_CLAMP_MIN_MAX(resultX, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultY, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultZ, 0.0f, 1.0f);
    }

    if (stageEnable.EOTFEnable)
    {
        if (param.EOTFGamma == VPHAL_GAMMA_TRADITIONAL_GAMMA)
        {
            if (resultX < 0.081)
            {
                resultX = resultX / 4.5;
            }
            else
            {
                resultX = (resultX + 0.099) / 1.099;
                resultX = pow(resultX, 1.0 / 0.45);
            }

            if (resultY < 0.081)
            {
                resultY = resultY / 4.5;
            }
            else
            {
                resultY = (resultY + 0.099) / 1.099;
                resultY = pow(resultY, 1.0 / 0.45);
            }

            if (resultZ < 0.081)
            {
                resultZ = resultZ / 4.5;
            }
            else
            {
                resultZ = (resultZ + 0.099) / 1.099;
                resultZ = pow(resultZ, 1.0 / 0.45);
            }
        }
        else if (param.EOTFGamma == VPHAL_GAMMA_SMPTE_ST2084)
        {
            double temp = 0;

            resultX = pow(resultX, 1.0f / m2);
            temp    = c2 - c3 * resultX;
            resultX = resultX > c1 ? resultX - c1 : 0;
            resultX = resultX / temp;
            resultX = pow(resultX, 1.0f / m1);

            resultY = pow(resultY, 1.0f / m2);
            temp    = c2 - c3 * resultY;
            resultY = resultY > c1 ? resultY - c1 : 0;
            resultY = resultY / temp;
            resultY = pow(resultY, 1.0f / m1);

            resultZ = pow(resultZ, 1.0f / m2);
            temp    = c2 - c3 * resultZ;
            resultZ = resultZ > c1 ? resultZ - c1 : 0;
            resultZ = resultZ / temp;
            resultZ = pow(resultZ, 1.0f / m1);
        }
        else if (param.EOTFGamma == VPHAL_GAMMA_BT1886)
        {
            if (resultX < -0.0f)
            {
                resultX = 0;
            }
            else
            {
                resultX = pow(resultX, 2.4);
            }

            if (resultY < -0.0f)
            {
                resultY = 0;
            }
            else
            {
                resultY = pow(resultY, 2.4);
            }

            if (resultZ < -0.0f)
            {
                resultZ = 0;
            }
            else
            {
                resultZ = pow(resultZ, 2.4);
            }
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        MOS_CLAMP_MIN_MAX(resultX, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultY, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultZ, 0.0f, 1.0f);
    }

    if (stageEnable.CCMEnable)
    {
        std::array<float, 12> matrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        // BT709 to BT2020 CCM
        if (param.CCM == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
        {
            matrix = {0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f};
        }
        // BT2020 to BT709 CCM
        else if (param.CCM == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
        {
            matrix = {1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f};
        }

        double tempX = resultX;
        double tempY = resultY;
        double tempZ = resultZ;

        resultX = matrix[0] * tempX + matrix[1] * tempY + matrix[2] * tempZ + matrix[3];
        resultY = matrix[4] * tempX + matrix[5] * tempY + matrix[6] * tempZ + matrix[7];
        resultZ = matrix[8] * tempX + matrix[9] * tempY + matrix[10] * tempZ + matrix[11];

        MOS_CLAMP_MIN_MAX(resultX, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultY, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultZ, 0.0f, 1.0f);
    }

    if (stageEnable.PWLFEnable)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(CaluclateToneMapping3DLut(param.HdrMode, resultX, resultY, resultZ, resultX, resultY, resultZ));

        MOS_CLAMP_MIN_MAX(resultX, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultY, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultZ, 0.0f, 1.0f);
    }

    if (stageEnable.CCMExt1Enable)
    {
        std::array<float, 12> matrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        // BT709 to BT2020 CCM
        if (param.CCMExt1 == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
        {
            matrix = {0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f};
        }
        // BT2020 to BT709 CCM
        else if (param.CCMExt1 == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
        {
            matrix = {1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f};
        }
        else if (param.CCMExt1 == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                 param.CCMExt1 == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                 param.CCMExt1 == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(CalculateCCMWithMonitorGamut(param.CCMExt1, targetHdrParam, matrix));
        }

        double tempX = resultX;
        double tempY = resultY;
        double tempZ = resultZ;

        resultX = matrix[0] * resultX + matrix[1] * resultY + matrix[2] * resultZ + matrix[3];
        resultY = matrix[4] * resultX + matrix[5] * resultY + matrix[6] * resultZ + matrix[7];
        resultZ = matrix[8] * resultX + matrix[9] * resultY + matrix[10] * resultZ + matrix[11];

        MOS_CLAMP_MIN_MAX(resultX, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultY, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultZ, 0.0f, 1.0f);
    }

    if (stageEnable.CCMExt2Enable)
    {
        std::array<float, 12> matrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        // BT709 to BT2020 CCM
        if (param.CCMExt2 == VPHAL_HDR_CCM_BT601_BT709_TO_BT2020_MATRIX)
        {
            matrix = {0.627404078626f, 0.329282097415f, 0.043313797587f, 0.000000f, 0.069097233123f, 0.919541035593f, 0.011361189924f, 0.000000f, 0.016391587664f, 0.088013255546f, 0.895595009604f, 0.000000f};
        }
        // BT2020 to BT709 CCM
        else if (param.CCMExt2 == VPHAL_HDR_CCM_BT2020_TO_BT601_BT709_MATRIX)
        {
            matrix = {1.660490254890140f, -0.587638564717282f, -0.072851975229213f, 0.000000f, -0.124550248621850f, 1.132898753013895f, -0.008347895599309f, 0.000000f, -0.018151059958635f, -0.100578696221493f, 1.118729865913540f, 0.000000f};
        }
        else if (param.CCMExt2 == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX ||
                 param.CCMExt2 == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX ||
                 param.CCMExt2 == VPHAL_HDR_CCM_MONITOR_TO_BT709_MATRIX)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(CalculateCCMWithMonitorGamut(param.CCMExt2, targetHdrParam, matrix));
        }

        double tempX = resultX;
        double tempY = resultY;
        double tempZ = resultZ;

        resultX = matrix[0] * tempX + matrix[1] * tempY + matrix[2] * tempZ + matrix[3];
        resultY = matrix[4] * tempX + matrix[5] * tempY + matrix[6] * tempZ + matrix[7];
        resultZ = matrix[8] * tempX + matrix[9] * tempY + matrix[10] * tempZ + matrix[11];

        MOS_CLAMP_MIN_MAX(resultX, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultY, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultZ, 0.0f, 1.0f);
    }

    if (param.OETFGamma != VPHAL_GAMMA_NONE)
    {
        if (param.OETFGamma == VPHAL_GAMMA_TRADITIONAL_GAMMA)
        {
            if (resultX < 0.018)
            {
                resultX = 4.5 * resultX;
            }
            else
            {
                resultX = pow(resultX, 0.45);
                resultX = 1.099 * resultX - 0.099;
            }

            if (resultY < 0.018)
            {
                resultY = 4.5 * resultY;
            }
            else
            {
                resultY = pow(resultY, 0.45);
                resultY = 1.099 * resultY - 0.099;
            }

            if (resultZ < 0.018)
            {
                resultZ = 4.5 * resultZ;
            }
            else
            {
                resultZ = pow(resultZ, 0.45);
                resultZ = 1.099 * resultZ - 0.099;
            }
        }
        else if (param.OETFGamma == VPHAL_GAMMA_SMPTE_ST2084)
        {
            resultX = pow(resultX, m1);
            resultX = (c1 + c2 * resultX) / (1 + c3 * resultX);
            resultX = pow(resultX, m2);

            resultY = pow(resultY, m1);
            resultY = (c1 + c2 * resultY) / (1 + c3 * resultY);
            resultY = pow(resultY, m2);

            resultZ = pow(resultZ, m1);
            resultZ = (c1 + c2 * resultZ) / (1 + c3 * resultZ);
            resultZ = pow(resultZ, m2);
        }
        else if (param.OETFGamma == VPHAL_GAMMA_SRGB)
        {
            if (resultX < 0.0031308f)
            {
                resultX = 12.92 * resultX;
            }
            else
            {
                resultX = pow(resultX, (double)(1.0f / 2.4f));
                resultX = 1.055 * resultX - 0.055;
            }

            if (resultY < 0.0031308f)
            {
                resultY = 12.92 * resultY;
            }
            else
            {
                resultY = pow(resultY, (double)(1.0f / 2.4f));
                resultY = 1.055 * resultY - 0.055;
            }

            if (resultZ < 0.0031308f)
            {
                resultZ = 12.92 * resultZ;
            }
            else
            {
                resultZ = pow(resultZ, (double)(1.0f / 2.4f));
                resultZ = 1.055 * resultZ - 0.055;
            }
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        MOS_CLAMP_MIN_MAX(resultX, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultY, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultZ, 0.0f, 1.0f);
    }

    // OETF will output RGB surface
    // So if post CSC is needed, it will always be RGB to YUV conversion
    if (param.PostCSC != VPHAL_HDR_CSC_NONE)
    {
        float postCscMatrix[12] = {};
        VP_PUBLIC_CHK_STATUS_RETURN(CalculateCscMatrix(param.PostCSC, true, postCscMatrix));

        double tempX = resultX;
        double tempY = resultY;
        double tempZ = resultZ;

        resultX = postCscMatrix[0] * tempX + postCscMatrix[1] * tempY + postCscMatrix[2] * tempZ + postCscMatrix[3];
        resultY = postCscMatrix[4] * tempX + postCscMatrix[5] * tempY + postCscMatrix[6] * tempZ + postCscMatrix[7];
        resultZ = postCscMatrix[8] * tempX + postCscMatrix[9] * tempY + postCscMatrix[10] * tempZ + postCscMatrix[11];

        MOS_CLAMP_MIN_MAX(resultX, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultY, 0.0f, 1.0f);
        MOS_CLAMP_MIN_MAX(resultZ, 0.0f, 1.0f);
    }

    // Convert and round up the [0, 1] float color value to 16 bit integer value
    outputX = (uint16_t)(resultX * 65535.0f + 0.5f);
    outputY = (uint16_t)(resultY * 65535.0f + 0.5f);
    outputZ = (uint16_t)(resultZ * 65535.0f + 0.5f);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::Generate2SegmentsOETFLUT(float fStretchFactor, pfnOETFFunc oetfFunc, uint16_t *lut)
{
    int i = 0, j = 0;

    for (i = 0; i < VPHAL_HDR_OETF_1DLUT_HEIGHT; ++i)
    {
        for (j = 0; j < VPHAL_HDR_OETF_1DLUT_WIDTH; ++j)
        {
            int   idx = j + i * (VPHAL_HDR_OETF_1DLUT_WIDTH - 1);
            float a   = (idx < 32) ? ((1.0f / 1024.0f) * idx) : ((1.0f / 32.0f) * (idx - 31));

            if (a > 1.0f)
                a = 1.0f;

            a *= fStretchFactor;
            lut[i * VPHAL_HDR_OETF_1DLUT_WIDTH + j] = VpHal_FloatToHalfFloat(oetfFunc(a));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::CaluclateToneMapping3DLut(VPHAL_HDR_MODE hdrMode, double inputX, double inputY, double inputZ, double &outputX, double &outputY, double &outputZ)
{
    static auto ToneMapping = [](double pivot[5], double slope[6], double intercept[6], double input, double &output) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(pivot);
        VP_PUBLIC_CHK_NULL_RETURN(slope);
        VP_PUBLIC_CHK_NULL_RETURN(intercept);

        if (input < pivot[0])
        {
            output = input * slope[0] + intercept[0];
        }
        else if (input < pivot[1])
        {
            output = input * slope[1] + intercept[1];
        }
        else if (input < pivot[2])
        {
            output = input * slope[2] + intercept[2];
        }
        else if (input < pivot[3])
        {
            output = input * slope[3] + intercept[3];
        }
        else if (input < pivot[4])
        {
            output = input * slope[4] + intercept[4];
        }
        else
        {
            output = input * slope[5] + intercept[5];
        }
        if (output > 1.0f)
        {
            output = 1.0f;
        } 
        return MOS_STATUS_SUCCESS;
    };

    if (hdrMode == VPHAL_HDR_MODE_TONE_MAPPING || hdrMode == VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE)
    {
        double pivot[5] = {VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X1, VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X2, VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X3, VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X4, VPHAL_HDR_TONE_MAPPING_PIVOT_POINT_X5};
        double slope[6]     = {VPHAL_HDR_TONE_MAPPING_SLOPE0, VPHAL_HDR_TONE_MAPPING_SLOPE1, VPHAL_HDR_TONE_MAPPING_SLOPE2, VPHAL_HDR_TONE_MAPPING_SLOPE3, VPHAL_HDR_TONE_MAPPING_SLOPE4, VPHAL_HDR_TONE_MAPPING_SLOPE5};
        double intercept[6] = {VPHAL_HDR_TONE_MAPPING_INTERCEPT0, VPHAL_HDR_TONE_MAPPING_INTERCEPT1, VPHAL_HDR_TONE_MAPPING_INTERCEPT2, VPHAL_HDR_TONE_MAPPING_INTERCEPT3, VPHAL_HDR_TONE_MAPPING_INTERCEPT4, VPHAL_HDR_TONE_MAPPING_INTERCEPT5};

        VP_PUBLIC_CHK_STATUS_RETURN(ToneMapping(pivot, slope, intercept, inputX, outputX));
        VP_PUBLIC_CHK_STATUS_RETURN(ToneMapping(pivot, slope, intercept, inputY, outputY));
        VP_PUBLIC_CHK_STATUS_RETURN(ToneMapping(pivot, slope, intercept, inputZ, outputZ));
    }
    else if (hdrMode == VPHAL_HDR_MODE_INVERSE_TONE_MAPPING)
    {
        // For inverse tone mapping in 3D LUT
        // PWLF function should not be applied
        // A simple / 100 devision should be performed
        outputX = inputX / 100.0f;
        outputY = inputY / 100.0f;
        outputZ = inputZ / 100.0f;
    }
    else if (hdrMode == VPHAL_HDR_MODE_H2H)
    {
        // TODO: port the H2H mapping algorithm
        outputX = inputX;
        outputY = inputY;
        outputZ = inputZ;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::CalculateCCMWithMonitorGamut(VPHAL_HDR_CCM_TYPE ccmType, HDR_PARAMS &targetHdrParam, std::array<float, 12> &outMatrix)
{
    static auto Mat3Inverse = [](const Mat3 input, Mat3 output) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(input);
        VP_PUBLIC_CHK_NULL_RETURN(output);

        const float a0 = input[0][0];
        const float a1 = input[0][1];
        const float a2 = input[0][2];

        const float b0 = input[1][0];
        const float b1 = input[1][1];
        const float b2 = input[1][2];

        const float c0 = input[2][0];
        const float c1 = input[2][1];
        const float c2 = input[2][2];

        float det = a0 * (b1 * c2 - b2 * c1) + a1 * (b2 * c0 - b0 * c2) + a2 * (b0 * c1 - b1 * c0);

        if (det != 0.0f)
        {
            float det_recip = 1.0f / det;

            output[0][0] = (b1 * c2 - b2 * c1) * det_recip;
            output[0][1] = (a2 * c1 - a1 * c2) * det_recip;
            output[0][2] = (a1 * b2 - a2 * b1) * det_recip;

            output[1][0] = (b2 * c0 - b0 * c2) * det_recip;
            output[1][1] = (a0 * c2 - a2 * c0) * det_recip;
            output[1][2] = (a2 * b0 - a0 * b2) * det_recip;

            output[2][0] = (b0 * c1 - b1 * c0) * det_recip;
            output[2][1] = (a1 * c0 - a0 * c1) * det_recip;
            output[2][2] = (a0 * b1 - a1 * b0) * det_recip;
        }
        else
        {
            // irreversible
            output[0][0] = 1.0f;
            output[0][1] = 0.0f;
            output[0][2] = 0.0f;
            output[1][0] = 0.0f;
            output[1][1] = 1.0f;
            output[1][2] = 0.0f;
            output[2][0] = 0.0f;
            output[2][1] = 0.0f;
            output[2][2] = 1.0f;
        }

        return MOS_STATUS_SUCCESS;
    };

    static auto Mat3MultiplyVec3 = [](const Mat3 input, const Vec3 vec, Vec3 output) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(input);
        VP_PUBLIC_CHK_NULL_RETURN(vec);
        VP_PUBLIC_CHK_NULL_RETURN(output);
        output[0] = input[0][0] * vec[0] + input[0][1] * vec[1] + input[0][2] * vec[2];
        output[1] = input[1][0] * vec[0] + input[1][1] * vec[1] + input[1][2] * vec[2];
        output[2] = input[2][0] * vec[0] + input[2][1] * vec[1] + input[2][2] * vec[2];

        return MOS_STATUS_SUCCESS;
    };

    static auto RGB2CIEXYZMatrix = [](const float xr, const float yr, const float xg, const float yg, const float xb, const float yb, const float xn, const float yn, Mat3 output) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(output);

        const float zr = 1.0f - xr - yr;
        const float zg = 1.0f - xg - yg;
        const float zb = 1.0f - xb - yb;
        const float zn = 1.0f - xn - yn;

        // m * [ar, ag, ab]T = [xn / yn, 1.0f, zn / yn]T;
        const Mat3 m =
            {
                xr, xg, xb, yr, yg, yb, zr, zg, zb};

        Mat3 inversed_m;

        VP_PUBLIC_CHK_STATUS_RETURN(Mat3Inverse(m, inversed_m));

        const Vec3 XYZWithUnityY = {xn / yn, 1.0f, zn / yn};
        float      aragab[3];

        VP_PUBLIC_CHK_STATUS_RETURN(Mat3MultiplyVec3(inversed_m, XYZWithUnityY, aragab));

        output[0][0] = m[0][0] * aragab[0];
        output[1][0] = m[1][0] * aragab[0];
        output[2][0] = m[2][0] * aragab[0];
        output[0][1] = m[0][1] * aragab[1];
        output[1][1] = m[1][1] * aragab[1];
        output[2][1] = m[2][1] * aragab[1];
        output[0][2] = m[0][2] * aragab[2];
        output[1][2] = m[1][2] * aragab[2];
        output[2][2] = m[2][2] * aragab[2];

        return MOS_STATUS_SUCCESS;
    };

    static auto Mat3MultiplyMat3 = [](const Mat3 left, const Mat3 right, Mat3 output) -> MOS_STATUS {
        VP_PUBLIC_CHK_NULL_RETURN(left);
        VP_PUBLIC_CHK_NULL_RETURN(right);
        VP_PUBLIC_CHK_NULL_RETURN(output);

        output[0][0] = left[0][0] * right[0][0] + left[0][1] * right[1][0] + left[0][2] * right[2][0];
        output[0][1] = left[0][0] * right[0][1] + left[0][1] * right[1][1] + left[0][2] * right[2][1];
        output[0][2] = left[0][0] * right[0][2] + left[0][1] * right[1][2] + left[0][2] * right[2][2];
        output[1][0] = left[1][0] * right[0][0] + left[1][1] * right[1][0] + left[1][2] * right[2][0];
        output[1][1] = left[1][0] * right[0][1] + left[1][1] * right[1][1] + left[1][2] * right[2][1];
        output[1][2] = left[1][0] * right[0][2] + left[1][1] * right[1][2] + left[1][2] * right[2][2];
        output[2][0] = left[2][0] * right[0][0] + left[2][1] * right[1][0] + left[2][2] * right[2][0];
        output[2][1] = left[2][0] * right[0][1] + left[2][1] * right[1][1] + left[2][2] * right[2][1];
        output[2][2] = left[2][0] * right[0][2] + left[2][1] * right[1][2] + left[2][2] * right[2][2];

        return MOS_STATUS_SUCCESS;
    };

    float src_xr = 1.0f, src_yr = 1.0f;
    float src_xg = 1.0f, src_yg = 1.0f;
    float src_xb = 1.0f, src_yb = 1.0f;
    float src_xn = 1.0f, src_yn = 1.0f;

    float dst_xr = 1.0f, dst_yr = 1.0f;
    float dst_xg = 1.0f, dst_yg = 1.0f;
    float dst_xb = 1.0f, dst_yb = 1.0f;
    float dst_xn = 1.0f, dst_yn = 1.0f;

    Mat3 srcMatrix        = {1.0f};
    Mat3 dstMatrix        = {1.0f};
    Mat3 dstMatrixInverse = {1.0f};
    Mat3 srcToDstMatrix   = {1.0f};

    if (ccmType == VPHAL_HDR_CCM_BT2020_TO_MONITOR_MATRIX)
    {
        src_xr = 0.708f;
        src_yr = 0.292f;
        src_xg = 0.170f;
        src_yg = 0.797f;
        src_xb = 0.131f;
        src_yb = 0.046f;
        src_xn = 0.3127f;
        src_yn = 0.3290f;

        dst_xr = targetHdrParam.display_primaries_x[2] / 50000.0f;
        dst_yr = targetHdrParam.display_primaries_y[2] / 50000.0f;
        dst_xg = targetHdrParam.display_primaries_x[0] / 50000.0f;
        dst_yg = targetHdrParam.display_primaries_y[0] / 50000.0f;
        dst_xb = targetHdrParam.display_primaries_x[1] / 50000.0f;
        dst_yb = targetHdrParam.display_primaries_y[1] / 50000.0f;
        dst_xn = targetHdrParam.white_point_x / 50000.0f;
        dst_yn = targetHdrParam.white_point_y / 50000.0f;
    }
    else if (ccmType == VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX)
    {
        src_xr = targetHdrParam.display_primaries_x[2] / 50000.0f;
        src_yr = targetHdrParam.display_primaries_y[2] / 50000.0f;
        src_xg = targetHdrParam.display_primaries_x[0] / 50000.0f;
        src_yg = targetHdrParam.display_primaries_y[0] / 50000.0f;
        src_xb = targetHdrParam.display_primaries_x[1] / 50000.0f;
        src_yb = targetHdrParam.display_primaries_y[1] / 50000.0f;
        src_xn = targetHdrParam.white_point_x / 50000.0f;
        src_yn = targetHdrParam.white_point_y / 50000.0f;

        dst_xr = 0.708f;
        dst_yr = 0.292f;
        dst_xg = 0.170f;
        dst_yg = 0.797f;
        dst_xb = 0.131f;
        dst_yb = 0.046f;
        dst_xn = 0.3127f;
        dst_yn = 0.3290f;
    }
    else
    {
        // VPHAL_HDR_CCM_MONITOR_TO_BT2020_MATRIX
        src_xr = targetHdrParam.display_primaries_x[2] / 50000.0f;
        src_yr = targetHdrParam.display_primaries_y[2] / 50000.0f;
        src_xg = targetHdrParam.display_primaries_x[0] / 50000.0f;
        src_yg = targetHdrParam.display_primaries_y[0] / 50000.0f;
        src_xb = targetHdrParam.display_primaries_x[1] / 50000.0f;
        src_yb = targetHdrParam.display_primaries_y[1] / 50000.0f;
        src_xn = targetHdrParam.white_point_x / 50000.0f;
        src_yn = targetHdrParam.white_point_y / 50000.0f;

        dst_xr = 0.64f;
        dst_yr = 0.33f;
        dst_xg = 0.30f;
        dst_yg = 0.60f;
        dst_xb = 0.15f;
        dst_yb = 0.06f;
        dst_xn = 0.3127f;
        dst_yn = 0.3290f;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(RGB2CIEXYZMatrix(src_xr, src_yr, src_xg, src_yg, src_xb, src_yb, src_xn, src_yn, srcMatrix));
    VP_PUBLIC_CHK_STATUS_RETURN(RGB2CIEXYZMatrix(dst_xr, dst_yr, dst_xg, dst_yg, dst_xb, dst_yb, dst_xn, dst_yn, dstMatrix));

    VP_PUBLIC_CHK_STATUS_RETURN(Mat3Inverse(dstMatrix, dstMatrixInverse));
    VP_PUBLIC_CHK_STATUS_RETURN(Mat3MultiplyMat3(dstMatrixInverse, srcMatrix, srcToDstMatrix));

    outMatrix[0]  = srcToDstMatrix[0][0];
    outMatrix[1]  = srcToDstMatrix[0][1];
    outMatrix[2]  = srcToDstMatrix[0][2];
    outMatrix[3]  = 0.0f;
    outMatrix[4]  = srcToDstMatrix[1][0];
    outMatrix[5]  = srcToDstMatrix[1][1];
    outMatrix[6]  = srcToDstMatrix[1][2];
    outMatrix[7]  = 0.0f;
    outMatrix[8]  = srcToDstMatrix[2][0];
    outMatrix[9]  = srcToDstMatrix[2][1];
    outMatrix[10] = srcToDstMatrix[2][2];
    outMatrix[11] = 0.0f;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::CalculateCscMatrix(VPHAL_HDR_CSC_TYPE cscType, bool is3DLutPath, float *outMatrix)
{
    VP_PUBLIC_CHK_NULL_RETURN(outMatrix);

    if (cscType == VPHAL_HDR_CSC_YUV_TO_RGB_BT601)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetCscMatrixForHDR(CSpace_BT601, CSpace_sRGB, is3DLutPath, outMatrix));
    }
    else if (cscType == VPHAL_HDR_CSC_YUV_TO_RGB_BT709)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetCscMatrixForHDR(CSpace_BT709, CSpace_sRGB, is3DLutPath, outMatrix));
    }
    else if (cscType == VPHAL_HDR_CSC_YUV_TO_RGB_BT2020)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetCscMatrixForHDR(CSpace_BT2020, CSpace_sRGB, is3DLutPath, outMatrix));
    }
    else if (cscType == VPHAL_HDR_CSC_RGB_TO_YUV_BT601)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetCscMatrixForHDR(CSpace_sRGB, CSpace_BT601, is3DLutPath, outMatrix));
    }
    else if (cscType == VPHAL_HDR_CSC_RGB_TO_YUV_BT709)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetCscMatrixForHDR(CSpace_sRGB, CSpace_BT709, is3DLutPath, outMatrix));
    }
    else if (cscType == VPHAL_HDR_CSC_RGB_TO_YUV_BT2020)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetCscMatrixForHDR(CSpace_sRGB, CSpace_BT2020, is3DLutPath, outMatrix));
    }
    else if (cscType == VPHAL_HDR_CSC_RGB_TO_YUV_BT709_FULLRANGE)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(VpUtils::GetCscMatrixForHDR(CSpace_sRGB, CSpace_BT709_FullRange, is3DLutPath, outMatrix));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

HDR_FORMAT_DESCRIPTOR VpHdrLiteRenderFilter::GetFormatDescriptor(MOS_FORMAT Format)
{
    VP_FUNC_CALL();

    HDR_FORMAT_DESCRIPTOR FormatDescriptor = HDR_FORMAT_DESCRIPTOR_UNKNOW;

    switch (Format)
    {
        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
            FormatDescriptor = HDR_FORMAT_DESCRIPTOR_R10G10B10A2_UNORM;
            break;

        case Format_X8R8G8B8:
        case Format_A8R8G8B8:
        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
        case Format_AYUV:
            FormatDescriptor = HDR_FORMAT_DESCRIPTOR_R8G8B8A8_UNORM;
            break;

        case Format_NV12:
        case Format_NV21:
            FormatDescriptor = HDR_FORMAT_DESCRIPTOR_NV12;
            break;

        case Format_YUY2:
            FormatDescriptor = HDR_FORMAT_DESCRIPTOR_YUY2;
            break;

        case Format_P010:
            FormatDescriptor = HDR_FORMAT_DESCRIPTOR_P010;
            break;

        case Format_P016:
            FormatDescriptor = HDR_FORMAT_DESCRIPTOR_P016;
            break;

        case Format_A16R16G16B16F:
        case Format_A16B16G16R16F:
            FormatDescriptor = HDR_FORMAT_R16G16B16A16_FLOAT;
            break;

        default:
            VP_RENDER_ASSERTMESSAGE("Unsupported input format.");
            FormatDescriptor = HDR_FORMAT_DESCRIPTOR_UNKNOW;
            break;
    }

    return FormatDescriptor;
}

HDR_CHROMA_SITING VpHdrLiteRenderFilter::GetHdrChromaSiting(uint32_t ChromaSiting)
{
    VP_FUNC_CALL();

    HDR_CHROMA_SITING HdrChromaSiting = HDR_CHROMA_SITTING_A;

    switch (ChromaSiting)
    {
    case CHROMA_SITING_HORZ_LEFT:
        HdrChromaSiting = HDR_CHROMA_SITTING_A;
        break;
    default:
        HdrChromaSiting = HDR_CHROMA_SITTING_A;
        break;
    }

    return HdrChromaSiting;
}

HDR_ROTATION VpHdrLiteRenderFilter::GetHdrRotation(VPHAL_ROTATION Rotation)
{
    VP_FUNC_CALL();

    HDR_ROTATION HdrRotation = HDR_LAYER_ROTATION_0;

    switch (Rotation)
    {
    case VPHAL_ROTATION_IDENTITY:
        HdrRotation = HDR_LAYER_ROTATION_0;
        break;
    case VPHAL_ROTATION_90:
        HdrRotation = HDR_LAYER_ROTATION_90;
        break;
    case VPHAL_ROTATION_180:
        HdrRotation = HDR_LAYER_ROTATION_180;
        break;
    case VPHAL_ROTATION_270:
        HdrRotation = HDR_LAYER_ROTATION_270;
        break;
    case VPHAL_MIRROR_HORIZONTAL:
        HdrRotation = HDR_LAYER_MIRROR_H;
        break;
    case VPHAL_MIRROR_VERTICAL:
        HdrRotation = HDR_LAYER_MIRROR_V;
        break;
    case VPHAL_ROTATE_90_MIRROR_VERTICAL:
        HdrRotation = HDR_LAYER_ROT_90_MIR_V;
        break;
    case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
        HdrRotation = HDR_LAYER_ROT_90_MIR_H;
        break;
    default:
        HdrRotation = HDR_LAYER_ROTATION_0;
        break;
    }

    return HdrRotation;
}

MOS_STATUS VpHdrLiteRenderFilter::PopulateHdrLiteCurbeData(
    VpHdrLiteKrnStructedData                     &curbeData,
    const HDRLITE_HAL_PARAM                     &param,
    const VP_SURFACE_GROUP &surfaceGroup)
{
    VP_FUNC_CALL();

    // Initialize curbeData with zeros
    MOS_ZeroMemory(&curbeData, sizeof(VpHdrLiteKrnStructedData));

    // Set default values
    uint8_t wAlpha = 0xff;
    uint32_t uiSamplerStateIndex = 0;
    uint32_t uiSamplerStateIndex2 = 0;

    // Retrieve target surface
    auto targetIt = surfaceGroup.find(SurfaceTypeHdrTarget0);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(targetIt, &surfaceGroup);
    VP_SURFACE *targetSurf = targetIt->second;
    VP_RENDER_CHK_NULL_RETURN(targetSurf);
    VP_RENDER_CHK_NULL_RETURN(targetSurf->osSurface);

    // Validate layer count to prevent buffer overflow
    if (param.layer > VPHAL_MAX_HDR_INPUT_LAYER)
    {
        VP_RENDER_ASSERTMESSAGE("Layer count exceeds maximum allowed layers");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Process each input layer
    for (uint32_t i = 0; i < param.layer && i < VPHAL_MAX_HDR_INPUT_LAYER; ++i)
    {
        // Get rotation and scalingMode for this specific layer
        VPHAL_ROTATION rotation = param.inputLayerParam[i].rotation;
        VPHAL_SCALING_MODE scalingMode = param.inputLayerParam[i].scalingMode;
        // Retrieve source surface for this layer
        auto sourceIt = surfaceGroup.find(SurfaceType(SurfaceTypeHdrInputLayer0 + i));
        if (sourceIt == surfaceGroup.end())
        {
            VP_RENDER_ASSERTMESSAGE("Source surface not found for layer %d", i);
            return MOS_STATUS_INVALID_PARAMETER;
        }
        VP_SURFACE *pSource = sourceIt->second;
        VP_RENDER_CHK_NULL_RETURN(pSource);
        VP_RENDER_CHK_NULL_RETURN(pSource->osSurface);

        // Initialize local variables for this layer
        float fScaleX = 0.0f, fScaleY = 0.0f;
        float fStepX = 0.0f, fStepY = 0.0f;
        float fOriginX = 0.0f, fOriginY = 0.0f;
        float fShiftX = 0.0f, fShiftY = 0.0f;

        // Calculate scaling factors based on rotation
        if (rotation == VPHAL_ROTATION_IDENTITY ||
            rotation == VPHAL_ROTATION_180 ||
            rotation == VPHAL_MIRROR_HORIZONTAL ||
            rotation == VPHAL_MIRROR_VERTICAL)
        {
            fScaleX = (float)(pSource->rcDst.right - pSource->rcDst.left) /
                      (float)(pSource->rcSrc.right - pSource->rcSrc.left);
            fScaleY = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                      (float)(pSource->rcSrc.bottom - pSource->rcSrc.top);
        }
        else
        {
            // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
            // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
            fScaleX = (float)(pSource->rcDst.right - pSource->rcDst.left) /
                      (float)(pSource->rcSrc.bottom - pSource->rcSrc.top);
            fScaleY = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                      (float)(pSource->rcSrc.right - pSource->rcSrc.left);
        }

        // Adjust scaling mode if 1:1 scaling with bilinear
        VPHAL_SCALING_MODE currentScalingMode = scalingMode;
        if (fScaleX == 1.0f && fScaleY == 1.0f && scalingMode == VPHAL_SCALING_BILINEAR)
        {
            currentScalingMode = VPHAL_SCALING_NEAREST;
        }
        else if (scalingMode == VPHAL_SCALING_AVS)
        {
            currentScalingMode = VPHAL_SCALING_BILINEAR;
        }

        // Determine sampler state indices based on scaling mode
        // 
        //if (currentScalingMode == VPHAL_SCALING_AVS)
        //{
        //    uiSamplerStateIndex = VPHAL_HDR_AVS_SAMPLER_STATE_ADAPTIVE;
        //    if (pSource->osSurface->Format == Format_P010 ||
        //        pSource->osSurface->Format == Format_P016)
        //    {
        //        uiSamplerStateIndex2 = VPHAL_HDR_AVS_SAMPLER_STATE_ADAPTIVE;
        //    }
        //}
        if (currentScalingMode == VPHAL_SCALING_BILINEAR)
        {
            uiSamplerStateIndex = uiSamplerStateIndex2 = 0;
            //uiSamplerStateIndex = VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR;
            //if (pSource->osSurface->Format == Format_P010 ||
            //    pSource->osSurface->Format == Format_P016)
            //{
            //    uiSamplerStateIndex2 = VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR;
            //}
            fShiftX = VP_HW_LINEAR_SHIFT;
            fShiftY = VP_HW_LINEAR_SHIFT;
        }
        else
        {
            uiSamplerStateIndex = uiSamplerStateIndex2 = 1;
            //uiSamplerStateIndex = VPHAL_HDR_3D_SAMPLER_STATE_NEAREST;
            //if (pSource->osSurface->Format == Format_P010 ||
            //    pSource->osSurface->Format == Format_P016)
            //{
            //    uiSamplerStateIndex2 = VPHAL_HDR_3D_SAMPLER_STATE_BILINEAR;
            //}
            fShiftX = VP_HW_LINEAR_SHIFT;
            fShiftY = VP_HW_LINEAR_SHIFT;
        }

        // Calculate step ratios based on rotation
        if (rotation == VPHAL_ROTATION_IDENTITY ||
            rotation == VPHAL_ROTATION_180 ||
            rotation == VPHAL_MIRROR_HORIZONTAL ||
            rotation == VPHAL_MIRROR_VERTICAL)
        {
            fStepX = ((pSource->rcSrc.right - pSource->rcSrc.left) * 1.0f) /
                     ((pSource->rcDst.right - pSource->rcDst.left) > 0 ?
                      (pSource->rcDst.right - pSource->rcDst.left) : 1);
            fStepY = ((float)(pSource->rcSrc.bottom - pSource->rcSrc.top)) /
                     ((pSource->rcDst.bottom - pSource->rcDst.top) > 0 ?
                      (float)(pSource->rcDst.bottom - pSource->rcDst.top) : 1.0f);
        }
        else
        {
            // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
            // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
            fStepX = ((pSource->rcSrc.right - pSource->rcSrc.left) * 1.0f) /
                     ((pSource->rcDst.bottom - pSource->rcDst.top) > 0 ?
                      (pSource->rcDst.bottom - pSource->rcDst.top) : 1);
            fStepY = ((float)(pSource->rcSrc.bottom - pSource->rcSrc.top)) /
                     ((pSource->rcDst.right - pSource->rcDst.left) > 0 ?
                      (float)(pSource->rcDst.right - pSource->rcDst.left) : 1.0f);
        }

        // Calculate coordinate shifts based on rotation type
        uint32_t dwDestRectWidth = targetSurf->osSurface->dwWidth;
        uint32_t dwDestRectHeight = targetSurf->osSurface->dwHeight;

        switch (rotation)
        {
            case VPHAL_ROTATION_IDENTITY:
                fShiftX -= pSource->rcDst.left;
                fShiftY -= pSource->rcDst.top;
                break;
            case VPHAL_ROTATION_90:
                fShiftX -= (float)pSource->rcDst.top;
                fShiftY -= (float)dwDestRectWidth -
                          (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleX -
                          (float)pSource->rcDst.left;
                break;
            case VPHAL_ROTATION_180:
                fShiftX -= (float)dwDestRectWidth -
                          (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleX -
                          (float)pSource->rcDst.left;
                fShiftY -= (float)dwDestRectHeight -
                          (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleY -
                          (float)pSource->rcDst.top;
                break;
            case VPHAL_ROTATION_270:
                fShiftX -= (float)dwDestRectHeight -
                          (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleY -
                          (float)pSource->rcDst.top;
                fShiftY -= (float)pSource->rcDst.left;
                break;
            case VPHAL_MIRROR_HORIZONTAL:
                fShiftX -= (float)dwDestRectWidth -
                          (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleX -
                          (float)pSource->rcDst.left;
                fShiftY -= pSource->rcDst.top;
                break;
            case VPHAL_MIRROR_VERTICAL:
                fShiftX -= pSource->rcDst.left;
                fShiftY -= (float)dwDestRectHeight -
                          (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleY -
                          (float)pSource->rcDst.top;
                break;
            case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
                fShiftX -= (float)pSource->rcDst.top;
                fShiftY -= (float)pSource->rcDst.left;
                break;
            case VPHAL_ROTATE_90_MIRROR_VERTICAL:
            default:
                fShiftX -= (float)dwDestRectHeight -
                          (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleY -
                          (float)pSource->rcDst.top;
                fShiftY -= (float)dwDestRectWidth -
                          (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleX -
                          (float)pSource->rcDst.left;
                break;
        }

        // Calculate normalized origin coordinates
        fOriginX = ((float)pSource->rcSrc.left + fShiftX * fStepX) / 
                   (float)MOS_MIN(pSource->osSurface->dwWidth, (uint32_t)pSource->rcSrc.right);
        fOriginY = ((float)pSource->rcSrc.top + fShiftY * fStepY) / 
                   (float)MOS_MIN(pSource->osSurface->dwHeight, (uint32_t)pSource->rcSrc.bottom);

        // Normalize step ratios
        fStepX /= MOS_MIN(pSource->osSurface->dwWidth, (uint32_t)pSource->rcSrc.right);
        fStepY /= MOS_MIN(pSource->osSurface->dwHeight, (uint32_t)pSource->rcSrc.bottom);

        // Get format descriptor
        HDR_FORMAT_DESCRIPTOR FormatDescriptor = GetFormatDescriptor(pSource->osSurface->Format);
        if (FormatDescriptor == HDR_FORMAT_DESCRIPTOR_UNKNOW)
        {
            VP_RENDER_VERBOSEMESSAGE("Unsupported hdr input format");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Get chroma siting
        HDR_CHROMA_SITING ChromaSiting = GetHdrChromaSiting(pSource->ChromaSiting);

        // Determine channel swap
        bool bChannelSwap = false;
        if (pSource->osSurface->Format == Format_B10G10R10A2 ||
            pSource->osSurface->Format == Format_A8R8G8B8 ||
            pSource->osSurface->Format == Format_X8R8G8B8 ||
            pSource->osSurface->Format == Format_A16R16G16B16F)
        {
            bChannelSwap = true;
        }

        // Determine IEF bypass (always true for now, can be extended later)
        const bool bBypassIEF = true;

        // Get HDR rotation
        HDR_ROTATION HdrRotation = GetHdrRotation(rotation);

        // Determine 3D LUT flag
        bool b3dLut = (param.inputLayerParam[i].LUTMode == VPHAL_HDR_LUT_MODE_3D);

        // Fill curbeData arrays for this layer
        curbeData.horizontalFrameOrigin[i] = fOriginX;
        curbeData.verticalFrameOrigin[i] = fOriginY;
        curbeData.horizontalScalingStepRatio[i] = fStepX;
        curbeData.verticalScalingStepRatio[i] = fStepY;
        curbeData.topLeft[i].left= static_cast<uint16_t>(pSource->rcDst.left);
        curbeData.topLeft[i].top = static_cast<uint16_t>(pSource->rcDst.top);
        curbeData.bottomRight[i].right = static_cast<uint16_t>(pSource->rcDst.right - 1);
        curbeData.bottomRight[i].bottom = static_cast<uint16_t>(pSource->rcDst.bottom - 1);

        // Fill layer info bitfields
        curbeData.layerInfo[i].formatDescriptor = FormatDescriptor;
        curbeData.layerInfo[i].chromaSitting = ChromaSiting;
        curbeData.layerInfo[i].channelSwap = bChannelSwap ? 1 : 0;
        curbeData.layerInfo[i].iefBypass = bBypassIEF ? 1 : 0;
        curbeData.layerInfo[i].rotation = HdrRotation;
        curbeData.layerInfo[i].samplerIndexPlane1 = uiSamplerStateIndex;
        curbeData.layerInfo[i].samplerIndexPlane2_3 = uiSamplerStateIndex2;
        curbeData.layerInfo[i].ccmExtEnable = (param.inputLayerParam[i].stageEnable.CCMExt1Enable || 
                                               param.inputLayerParam[i].stageEnable.CCMExt2Enable) ? 1 : 0;
        curbeData.layerInfo[i].toneMappingEnable = param.inputLayerParam[i].stageEnable.PWLFEnable ? 1 : 0;
        curbeData.layerInfo[i].priorCscEnable = param.inputLayerParam[i].stageEnable.PriorCSCEnable ? 1 : 0;
        curbeData.layerInfo[i].eotfEnable = param.inputLayerParam[i].stageEnable.EOTFEnable ? 1 : 0;
        curbeData.layerInfo[i].ccmEnable = param.inputLayerParam[i].stageEnable.CCMEnable ? 1 : 0;
        curbeData.layerInfo[i].oetfEnable = param.inputLayerParam[i].stageEnable.OETFEnable ? 1 : 0;
        curbeData.layerInfo[i].postCscEnable = param.inputLayerParam[i].stageEnable.PostCSCEnable ? 1 : 0;
        curbeData.layerInfo[i].lut3dEnable = b3dLut ? 1 : 0;

        // Set two-layer operation
        HDR_TWO_LAYER_OPTION HdrTwoLayerOp = HDR_TWO_LAYER_OPTION_SBLEND;
        if (i == 0)
        {
            // For layer 0, check if it's primary layer with specific blending conditions
            curbeData.twoLayerOperation[i] = HDR_TWO_LAYER_OPTION_COMP;
            if (pSource->SurfType == SURF_IN_PRIMARY &&
                param.inputLayerParam[i].blending.BlendType == BLEND_SOURCE &&
                (IS_RGB_CSPACE(pSource->ColorSpace) || IS_COLOR_SPACE_BT2020_RGB(pSource->ColorSpace)))
            {
                curbeData.twoLayerOperation[i] = HDR_TWO_LAYER_OPTION_SBLEND;
            }
        }
        else
        {
            curbeData.twoLayerOperation[i] = HdrTwoLayerOp;
        }

    }

    // Handle destination surface configuration
    HDR_FORMAT_DESCRIPTOR FormatDescriptor = GetFormatDescriptor(targetSurf->osSurface->Format);
    HDR_CHROMA_SITING ChromaSiting = GetHdrChromaSiting(targetSurf->ChromaSiting);

    bool bChannelSwap = false;
    if (targetSurf->osSurface->Format == Format_B10G10R10A2 ||
        targetSurf->osSurface->Format == Format_A8R8G8B8 ||
        targetSurf->osSurface->Format == Format_X8R8G8B8 ||
        targetSurf->osSurface->Format == Format_A16R16G16B16F)
    {
        bChannelSwap = true;
    }

    // Fill destination fields
    curbeData.dstWidth  = static_cast<uint16_t> (targetSurf->osSurface->dwWidth);
    curbeData.dstHeight = static_cast<uint16_t> (targetSurf->osSurface->dwHeight);
    curbeData.layerNum = param.layer;

    // Handle zero-layer case - set coordinates off-screen
    constexpr uint16_t OFFSCREEN_COORDINATE_OFFSET = 16;
    if (param.layer == 0)
    {
        curbeData.topLeft[0].left = static_cast<uint16_t> (targetSurf->osSurface->dwWidth + OFFSCREEN_COORDINATE_OFFSET);
        curbeData.topLeft[0].top = static_cast<uint16_t> (targetSurf->osSurface->dwHeight + OFFSCREEN_COORDINATE_OFFSET);
        curbeData.bottomRight[0].right = static_cast<uint16_t> (targetSurf->osSurface->dwWidth + OFFSCREEN_COORDINATE_OFFSET);
        curbeData.bottomRight[0].bottom = static_cast<uint16_t> (targetSurf->osSurface->dwHeight + OFFSCREEN_COORDINATE_OFFSET);
        curbeData.twoLayerOperation[0] = HDR_TWO_LAYER_OPTION_COMP;
    }

    // Fill dstInfo bitfields
    curbeData.dstInfo.formatDescriptor = FormatDescriptor;
    curbeData.dstInfo.chromaSitting = ChromaSiting;
    curbeData.dstInfo.channelSwap = bChannelSwap ? 1 : 0;
    curbeData.dstInfo.dstCscEnable = 0;
    curbeData.dstInfo.reserved = 0;
    curbeData.dstInfo.ditherRoundEnable = 0;

    // Handle background fill color if enabled
    if (param.enableColorFill)
    {
        VPHAL_COLOR_SAMPLE_8 Src, Dst;
        VPHAL_CSPACE src_cspace, dst_cspace;

        Src.dwValue = param.colorFillParams.Color;
        src_cspace = param.colorFillParams.CSpace;
        dst_cspace = targetSurf->ColorSpace;

        // Convert BG color with color space conversion
        if (VpUtils::GetCscMatrixForRender8Bit(&Dst, &Src, src_cspace, dst_cspace))
        {
            curbeData.fillColor.rvChannel = Dst.R << 8;
            curbeData.fillColor.gyChannel = Dst.G << 8;
            curbeData.fillColor.buChannel = Dst.B << 8;
            curbeData.fillColor.alphaChannel = Dst.A << 8;
        }
        else
        {
            curbeData.fillColor.rvChannel = Src.R << 8;
            curbeData.fillColor.gyChannel = Src.G << 8;
            curbeData.fillColor.buChannel = Src.B << 8;
            curbeData.fillColor.alphaChannel = Src.A << 8;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::GenerateHdrKrnParam(
    const HDRLITE_HAL_PARAM                     &param,
    const VP_SURFACE_GROUP &surfaceGroup,
    RENDER_HDRLITE_KERNEL_PARAM                 &renderParam)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface); 

    renderParam = {};
    
    // Populate CURBE data structure
    VpHdrLiteKrnStructedData curbeData;
    VP_RENDER_CHK_STATUS_RETURN(PopulateHdrLiteCurbeData(curbeData, param, surfaceGroup));

    PrintHdrLiteCurbeData(curbeData);

    // Retrieve kernel configuration from kernel pool
    auto handle = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find("HDR_mandatory_HdrRender");
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

    KERNEL_ARGS                  krnArgs             = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces = {};

    // Process kernel arguments
    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = m_hdrMandatoryKrnArgs.find(uIndex);
        if (argHandle == m_hdrMandatoryKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = m_hdrMandatoryKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_hdrMandatoryKrnArgs);
        }
        
        KRN_ARG &krnArg = argHandle->second;
        bool     bInit  = true;
        krnArg.uIndex   = uIndex;
        krnArg.eArgKind = kernelArg.eArgKind;
        
        if (krnArg.pData == nullptr)
        {
            if (kernelArg.uSize > 0)
            {
                krnArg.uSize = kernelArg.uSize;
                krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
            }
        }
        else
        {
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
            MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
        }

        if (kernelArg.eArgKind == ARG_KIND_SURFACE)
        {
            SURFACE_PARAMS surfaceParam = {};
            VP_RENDER_CHK_STATUS_RETURN(SetupHdrStatefulSurface(uIndex, param, surfaceGroup, surfaceParam));
            krnStatefulSurfaces.emplace(uIndex, surfaceParam);
        }
        else
        {
            VP_RENDER_CHK_STATUS_RETURN(SetupHdrKrnArg(curbeData, krnArg, bInit));

            if (bInit)
            {
                krnArgs.push_back(krnArg);
            }
        }
    }

    auto outputIt = surfaceGroup.find(SurfaceTypeHdrTarget0);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(outputIt, &surfaceGroup);
    VP_SURFACE *outputSurf = outputIt->second;
    VP_RENDER_CHK_NULL_RETURN(outputSurf);
    VP_RENDER_CHK_NULL_RETURN(outputSurf->osSurface);

    // Configure render parameter
    renderParam.kernelID               = VpKernelID(kernelHdrMandatoryLite);
    renderParam.kernelArgs             = krnArgs;
    renderParam.kernelStatefulSurfaces = krnStatefulSurfaces;
    renderParam.localHeight            = 1;
    renderParam.localWidth             = 1;
    renderParam.threadDepth            = 1;
    renderParam.threadHeight           = MOS_ALIGN_CEIL(outputSurf->rcSrc.bottom - outputSurf->rcDst.top, 8) / 8;
    renderParam.threadWidth            = MOS_ALIGN_CEIL(outputSurf->rcDst.right - outputSurf->rcDst.left, 16) / 16; 

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::SetupHdrKrnArg(
    const VpHdrLiteKrnStructedData &curbeData,
    KRN_ARG                     &krnArg,
    bool                        &bInit)
{
    VP_FUNC_CALL();

    // Now populate the data based on the argument type
    switch (krnArg.uIndex)
    {
    case HDRRENDER_HDR_MANDATORY_START_X:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, curbeData.horizontalFrameOrigin, sizeof(curbeData.horizontalFrameOrigin));
        break;

    case HDRRENDER_HDR_MANDATORY_START_Y:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, curbeData.verticalFrameOrigin, sizeof(curbeData.verticalFrameOrigin));
        break;

    case HDRRENDER_HDR_MANDATORY_DELTA_X:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, curbeData.horizontalScalingStepRatio, sizeof(curbeData.horizontalScalingStepRatio));
        break;

    case HDRRENDER_HDR_MANDATORY_DELTA_Y:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, curbeData.verticalScalingStepRatio, sizeof(curbeData.verticalScalingStepRatio));
        break;

    case HDRRENDER_HDR_MANDATORY_TOP_LEFT:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, curbeData.topLeft, sizeof(curbeData.topLeft));
        break;

    case HDRRENDER_HDR_MANDATORY_BOTTOM_RIGHT:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, curbeData.bottomRight, sizeof(curbeData.bottomRight));
        break;

    case HDRRENDER_HDR_MANDATORY_LAYER_INFO:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, curbeData.layerInfo, sizeof(curbeData.layerInfo));
        break;

    case HDRRENDER_HDR_MANDATORY_CONST_ALPHA:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, curbeData.constantAlpha, sizeof(curbeData.constantAlpha));
        break;

    case HDRRENDER_HDR_MANDATORY_OP_TYPE:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, curbeData.twoLayerOperation, sizeof(curbeData.twoLayerOperation));
        break;

    case HDRRENDER_HDR_MANDATORY_FILL_COLOR:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &curbeData.fillColor, sizeof(curbeData.fillColor));
        break;

    case HDRRENDER_HDR_MANDATORY_DST_WIDTH:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &curbeData.dstWidth, sizeof(curbeData.dstWidth));
        break;

    case HDRRENDER_HDR_MANDATORY_DST_HEIGHT:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &curbeData.dstHeight, sizeof(curbeData.dstHeight));
        break;

    case HDRRENDER_HDR_MANDATORY_LAYER_NUM:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &curbeData.layerNum, sizeof(curbeData.layerNum));
        break;

    case HDRRENDER_HDR_MANDATORY_DST_INFO:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &curbeData.dstInfo, sizeof(curbeData.dstInfo));
        break;

    case HDRRENDER_HDR_MANDATORY_SAMPLE0:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        {
            uint8_t samplerFilter0 = MHW_SAMPLER_FILTER_BILINEAR;
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &samplerFilter0, sizeof(samplerFilter0));
        }
        break;
    case HDRRENDER_HDR_MANDATORY_SAMPLE1:
        VP_RENDER_CHK_NULL_RETURN(krnArg.pData);
        {
            uint8_t samplerFilter1 = MHW_SAMPLER_FILTER_NEAREST;
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &samplerFilter1, sizeof(samplerFilter1));
        }
        break;
    default:
        // Skip non-CURBE arguments
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrLiteRenderFilter::SetupHdrStatefulSurface(
    uint32_t                                  uIndex,
    const HDRLITE_HAL_PARAM                     &param,
    const VP_SURFACE_GROUP &surfaceGroup,
    SURFACE_PARAMS                           &surfaceParam)
{
    VP_FUNC_CALL();
    
    // Map kernel surface indices to SurfaceType
    switch (uIndex)
    {
        // Input layer surfaces (SURF0-SURF39 map to 8 layers x 5 planes each)
        // Layer 0 planes
        case HDRRENDER_HDR_MANDATORY_SURF0:  // Layer 0, Plane 0 (Y or RGB)
            if (param.layer > 0)
            {
                surfaceParam.surfType = SurfaceTypeHdrInputLayer0;
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF1:  // Layer 0, Plane 1 (UV)
            if (param.layer > 0)
            {
                surfaceParam.surfType = SurfaceTypeHdrInputLayer0;
                surfaceParam.planeIndex = 1;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF2:  // Layer 0, Plane 2 (V for planar)
            if (param.layer > 0)
            {
                surfaceParam.surfType = SurfaceTypeHdrInputLayer0;
                surfaceParam.planeIndex = 2;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF3:  // Layer 0, Cri3DLut
            if (param.layer > 0 && surfaceGroup.find(SurfaceTypeHdrCRI3DLUTSurface0) != surfaceGroup.end())
            {
                surfaceParam.surfType   = SurfaceTypeHdrCRI3DLUTSurface0;
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF4:  // Layer 0, OETF 1D LUT
            if (param.layer > 0 && surfaceGroup.find(SurfaceTypeHdrOETF1DLUTSurface0) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceTypeHdrOETF1DLUTSurface0;
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
            
        // Layer 1 planes
        case HDRRENDER_HDR_MANDATORY_SURF5:  // Layer 1, Plane 0
            if (param.layer > 1)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 1);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF6:  // Layer 1, Plane 1
            if (param.layer > 1)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 1);
                surfaceParam.planeIndex = 1;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF7:  // Layer 1, Plane 2
            if (param.layer > 1)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 1);
                surfaceParam.planeIndex = 2;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF8:  // Layer 1, Cri3DLut
            if (param.layer > 1 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 1)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 1);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF9:  // Layer 1, OETF 1D LUT
            if (param.layer > 1 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 1)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 1);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
            
        // Layer 2 planes
        case HDRRENDER_HDR_MANDATORY_SURF10:  // Layer 2, Plane 0
            if (param.layer > 2)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 2);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF11:  // Layer 2, Plane 1
            if (param.layer > 2)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 2);
                surfaceParam.planeIndex = 1;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF12:  // Layer 2, Plane 2
            if (param.layer > 2)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 2);
                surfaceParam.planeIndex = 2;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF13:  // Layer 2, Cri3DLut
            if (param.layer > 2 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 2)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 2);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF14:  // Layer 2, OETF 1D LUT
            if (param.layer > 2 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 2)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 2);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
            
        // Layer 3 planes
        case HDRRENDER_HDR_MANDATORY_SURF15:  // Layer 3, Plane 0
            if (param.layer > 3)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 3);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF16:  // Layer 3, Plane 1
            if (param.layer > 3)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 3);
                surfaceParam.planeIndex = 1;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF17:  // Layer 3, Plane 2
            if (param.layer > 3)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 3);
                surfaceParam.planeIndex = 2;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF18:  // Layer 3, Cri3DLut
            if (param.layer > 3 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 3)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 3);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF19:  // Layer 3, OETF 1D LUT
            if (param.layer > 3 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 3)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 3);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
            
        // Layer 4 planes
        case HDRRENDER_HDR_MANDATORY_SURF20:  // Layer 4, Plane 0
            if (param.layer > 4)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 4);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF21:  // Layer 4, Plane 1
            if (param.layer > 4)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 4);
                surfaceParam.planeIndex = 1;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF22:  // Layer 4, Plane 2
            if (param.layer > 4)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 4);
                surfaceParam.planeIndex = 2;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF23:  // Layer 4, Cri3DLut
            if (param.layer > 4 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 4)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 4);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF24:  // Layer 4, OETF 1D LUT
            if (param.layer > 4 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 4)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 4);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
            
        // Layer 5 planes
        case HDRRENDER_HDR_MANDATORY_SURF25:  // Layer 5, Plane 0
            if (param.layer > 5)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 5);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF26:  // Layer 5, Plane 1
            if (param.layer > 5)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 5);
                surfaceParam.planeIndex = 1;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF27:  // Layer 5, Plane 2
            if (param.layer > 5)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 5);
                surfaceParam.planeIndex = 2;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF28:  // Layer 5, Cri3DLut
            if (param.layer > 5 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 5)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 5);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF29:  // Layer 5, OETF 1D LUT
            if (param.layer > 5 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 5)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 5);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
            
        // Layer 6 planes
        case HDRRENDER_HDR_MANDATORY_SURF30:  // Layer 6, Plane 0
            if (param.layer > 6)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 6);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF31:  // Layer 6, Plane 1
            if (param.layer > 6)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 6);
                surfaceParam.planeIndex = 1;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF32:  // Layer 6, Plane 2
            if (param.layer > 6)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 6);
                surfaceParam.planeIndex = 2;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF33:  // Layer 6, Cri3DLut
            if (param.layer > 6 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 6)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 6);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF34:  // Layer 6, OETF 1D LUT
            if (param.layer > 6 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 6)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 6);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
            
        // Layer 7 planes
        case HDRRENDER_HDR_MANDATORY_SURF35:  // Layer 7, Plane 0
            if (param.layer > 7)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 7);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF36:  // Layer 7, Plane 1
            if (param.layer > 7)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 7);
                surfaceParam.planeIndex = 1;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF37:  // Layer 7, Plane 2
            if (param.layer > 7)
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrInputLayer0 + 7);
                surfaceParam.planeIndex = 2;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF38:  // Layer 7, Cri3DLut
            if (param.layer > 7 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 7)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrCRI3DLUTSurface0 + 7);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
        case HDRRENDER_HDR_MANDATORY_SURF39:  // Layer 7, OETF 1D LUT
            if (param.layer > 7 && surfaceGroup.find(SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 7)) != surfaceGroup.end())
            {
                surfaceParam.surfType = SurfaceType(SurfaceTypeHdrOETF1DLUTSurface0 + 7);
                surfaceParam.planeIndex = 0;
            }
            else
            {
                surfaceParam.surfType = SurfaceTypeInvalid;
            }
            break;
            
        // Output surfaces
        case HDRRENDER_HDR_MANDATORY_DST_SURFACE:  // Output Y or RGB plane
            surfaceParam.surfType = SurfaceTypeHdrTarget0;
            surfaceParam.isOutput = true;
            surfaceParam.planeIndex = 0;
            break;
        case HDRRENDER_HDR_MANDATORY_DST_SURFACE_UV:  // Output UV plane
            surfaceParam.surfType = SurfaceTypeHdrTarget0;
            surfaceParam.isOutput = true;
            surfaceParam.planeIndex = 1;
            break;
        case HDRRENDER_HDR_MANDATORY_DST_SURFACE_V:  // Output V plane (for planar)
            surfaceParam.surfType = SurfaceTypeHdrTarget0;
            surfaceParam.isOutput = true;
            surfaceParam.planeIndex = 2;
            break;
            
        // Coefficient surface
        case HDRRENDER_HDR_MANDATORY_COEF_SURFACE:
            surfaceParam.surfType = SurfaceTypeHdrCoeff;
            surfaceParam.planeIndex = 0;
            break;
        default:
            surfaceParam.surfType = SurfaceTypeInvalid;
            break;
    }
    return MOS_STATUS_SUCCESS;
}
CSC_COEFF_FORMAT VpHdrLiteRenderFilter::ConvertDouble2RegisterForamt(double input)
{
    CSC_COEFF_FORMAT outVal      = {};
    uint32_t         shiftFactor = 0;

    if (input < 0)
    {
        outVal.sign = 1;
        input       = -input;
    }

    // range check
    if (input > MAX_CSC_COEFF_VAL_ICL)
        input = MAX_CSC_COEFF_VAL_ICL;

    if (input < 0.125)  //0.000bbbbbbbbb
    {
        outVal.exponent = 3;
        shiftFactor     = 12;
    }
    else if (input >= 0.125 && input < 0.25)  //0.00bbbbbbbbb
    {
        outVal.exponent = 2;
        shiftFactor     = 11;
    }
    else if (input >= 0.25 && input < 0.5)  //0.0bbbbbbbbb
    {
        outVal.exponent = 1;
        shiftFactor     = 10;
    }
    else if (input >= 0.5 && input < 1.0)  // 0.bbbbbbbbb
    {
        outVal.exponent = 0;
        shiftFactor     = 9;
    }
    else if (input >= 1.0 && input < 2.0)  //b.bbbbbbbb
    {
        outVal.exponent = 7;
        shiftFactor     = 8;
    }
    else if (input >= 2.0)  // bb.bbbbbbb
    {
        outVal.exponent = 6;
        shiftFactor    = 7;
    }

    //Convert float to integer
    outVal.mantissa = static_cast<uint32_t>(round(input * (double)(1 << (int)shiftFactor)));

    return outVal;
}

double VpHdrLiteRenderFilter::ConvertRegister2DoubleFormat(CSC_COEFF_FORMAT regVal)
{
    double outVal = 0;

    switch (regVal.exponent)
    {
    case 0:
        outVal = (double)regVal.mantissa / 512.0;
        break;
    case 1:
        outVal = (double)regVal.mantissa / 1024.0;
        break;
    case 2:
        outVal = (double)regVal.mantissa / 2048.0;
        break;
    case 3:
        outVal = (double)regVal.mantissa / 4096.0;
        break;
    case 6:
        outVal = (double)regVal.mantissa / 128.0;
        break;
    case 7:
        outVal = (double)regVal.mantissa / 256.0;
        break;
    }

    if (regVal.sign)
    {
        outVal = -outVal;
    }

    return outVal;
}

/****************************************************************************************************/
/*                                   HwFilter HdrLite Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterHdrLiteRenderParameter::Create(HW_FILTER_HDRLITE_RENDER_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterHdrLiteRenderParameter *p = MOS_New(HwFilterHdrLiteRenderParameter, featureType);
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

HwFilterHdrLiteRenderParameter::HwFilterHdrLiteRenderParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterHdrLiteRenderParameter::~HwFilterHdrLiteRenderParameter()
{
}

MOS_STATUS HwFilterHdrLiteRenderParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterHdrLiteRenderParameter::Initialize(HW_FILTER_HDRLITE_RENDER_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Render HdrLite Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpRenderHdrLiteParameter::Create(HW_FILTER_HDRLITE_RENDER_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpRenderHdrLiteParameter *p = dynamic_cast<VpRenderHdrLiteParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpRenderHdrLiteParameter::VpRenderHdrLiteParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) : VpPacketParameter(packetParamFactory), m_HdrFilter(pHwInterface)
{
}
VpRenderHdrLiteParameter::~VpRenderHdrLiteParameter() {}

bool VpRenderHdrLiteParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    RENDER_HDRLITE_PARAMS *pParams = m_HdrFilter.GetRenderParams();
    if (nullptr == pParams)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get render hdr params");
        return false;
    }

    VpRenderCmdPacket *packet = dynamic_cast<VpRenderCmdPacket *>(pPacket);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetHdrLiteParams(pParams));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for render hdr");
    return false;
}

MOS_STATUS VpRenderHdrLiteParameter::Initialize(HW_FILTER_HDRLITE_RENDER_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_HdrFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_HdrFilter.SetExecuteEngineCaps(params.executedPipe, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_HdrFilter.CalculateEngineParams(params.hdrParams, params.vpExecuteCaps));
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Render Hdr Handler                                         */
/****************************************************************************************************/
PolicyRenderHdrLiteHandler::PolicyRenderHdrLiteHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeHdrOnRender;
}
PolicyRenderHdrLiteHandler::~PolicyRenderHdrLiteHandler()
{
}

bool PolicyRenderHdrLiteHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bRenderHdr;
}

HwFilterParameter *PolicyRenderHdrLiteHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        SwFilterHdr *swFilter = dynamic_cast<SwFilterHdr *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeHdrOnRender));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamHdr &param = swFilter->GetSwFilterParams();

        HW_FILTER_HDRLITE_RENDER_PARAM paramHdr = {};
        paramHdr.type                        = m_Type;
        paramHdr.pHwInterface                = pHwInterface;
        paramHdr.vpExecuteCaps               = vpExecuteCaps;
        paramHdr.pPacketParamFactory         = &m_PacketParamFactory;
        paramHdr.hdrParams                   = param;
        paramHdr.executedPipe                = &swFilterPipe;
        paramHdr.pfnCreatePacketParam        = PolicyRenderHdrLiteHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterHdrLiteRenderParameter *)pHwFilterParam)->Initialize(paramHdr)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterHdrLiteRenderParameter::Create(paramHdr, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicyRenderHdrLiteHandler::LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps)
{
    for (uint32_t index = 0; index < featurePipe.GetSurfaceCount(true); ++index)
    {
        SwFilterSubPipe *subpipe = featurePipe.GetSwFilterSubPipe(true, index);
        VP_PUBLIC_CHK_NULL_RETURN(subpipe);

        SwFilterHdr *hdr = dynamic_cast<SwFilterHdr *>(subpipe->GetSwFilter(FeatureType::FeatureTypeHdr));
        if (nullptr == hdr)
        {
            continue;
        }

        SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(subpipe->GetSwFilter(FeatureType::FeatureTypeScaling));
        VP_PUBLIC_CHK_NULL_RETURN(scaling);

        // Disable AVS scaling mode
        if (!m_hwCaps.m_rules.isAvsSamplerSupported)
        {
            if (VPHAL_SCALING_AVS == scaling->GetSwFilterParams().scalingMode)
            {
                scaling->GetSwFilterParams().scalingMode = VPHAL_SCALING_BILINEAR;
            }
        }
        layerIndexes.push_back(index);
    }

    return MOS_STATUS_SUCCESS;
}

void VpHdrLiteRenderFilter::PrintHdrLiteCurbeData(const VpHdrLiteKrnStructedData &curbeData)
{
    uint32_t hexVal = 0;

    for (int i = 0; i < 8; i++)
    {
        MOS_SecureMemcpy(&hexVal, sizeof(uint32_t), &curbeData.horizontalFrameOrigin[i], sizeof(uint32_t));
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] HorizontalFrameOrigin[%d] = %f (0x%08x)", i, curbeData.horizontalFrameOrigin[i], hexVal);
        MOS_SecureMemcpy(&hexVal, sizeof(uint32_t), &curbeData.verticalFrameOrigin[i], sizeof(uint32_t));
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] VerticalFrameOrigin[%d] = %f (0x%08x)", i, curbeData.verticalFrameOrigin[i], hexVal);
        MOS_SecureMemcpy(&hexVal, sizeof(uint32_t), &curbeData.horizontalScalingStepRatio[i], sizeof(uint32_t));
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] HorizontalScalingStepRatio[%d] = %f (0x%08x)", i, curbeData.horizontalScalingStepRatio[i], hexVal);
        MOS_SecureMemcpy(&hexVal, sizeof(uint32_t), &curbeData.verticalScalingStepRatio[i], sizeof(uint32_t));
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] VerticalScalingStepRatio[%d] = %f (0x%08x)", i, curbeData.verticalScalingStepRatio[i], hexVal);

        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] LeftCoordinateRectangle[%d] = %d", i, curbeData.topLeft[i].left);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] TopCoordinateRectangle[%d] = %d", i, curbeData.topLeft[i].top);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] RightCoordinateRectangle[%d] = %d", i, curbeData.bottomRight[i].right);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] BottomCoordinateRectangle[%d] = %d", i, curbeData.bottomRight[i].bottom);

        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] FormatDescriptor[%d] = %d", i, curbeData.layerInfo[i].formatDescriptor);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] ChromaSittingLocation[%d] = %d", i, curbeData.layerInfo[i].chromaSitting);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] ChannelSwapEnablingFlag[%d] = %d", i, curbeData.layerInfo[i].channelSwap);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] IEFBypassEnablingFlag[%d] = %d", i, curbeData.layerInfo[i].iefBypass);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] RotationAngleMirrorDirection[%d] = %d", i, curbeData.layerInfo[i].rotation);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] SamplerIndexFirstPlane[%d] = %d", i, curbeData.layerInfo[i].samplerIndexPlane1);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] SamplerIndexSecondThirdPlane[%d] = %d", i, curbeData.layerInfo[i].samplerIndexPlane2_3);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] CCMExtensionEnablingFlag[%d] = %d", i, curbeData.layerInfo[i].ccmExtEnable);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] ToneMappingEnablingFlag[%d] = %d", i, curbeData.layerInfo[i].toneMappingEnable);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] PriorCSCEnablingFlag[%d] = %d", i, curbeData.layerInfo[i].priorCscEnable);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] EOTF1DLUTEnablingFlag[%d] = %d", i, curbeData.layerInfo[i].eotfEnable);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] CCMEnablingFlag[%d] = %d", i, curbeData.layerInfo[i].ccmEnable);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] OETF1DLUTEnablingFlag[%d] = %d", i, curbeData.layerInfo[i].oetfEnable);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] PostCSCEnablingFlag[%d] = %d", i, curbeData.layerInfo[i].postCscEnable);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] Enabling3DLUTFlag[%d] = %d", i, curbeData.layerInfo[i].lut3dEnable);

        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] ConstantBlendingAlpha[%d] = %d", i, curbeData.constantAlpha[i]);
        VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] TwoLayerOperation[%d] = %d", i, curbeData.twoLayerOperation[i]);
    }

    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] FillColorRV = %d", curbeData.fillColor.rvChannel);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] FillColorGY = %d", curbeData.fillColor.gyChannel);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] FillColorBU = %d", curbeData.fillColor.buChannel);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] FillColorAlpha = %d", curbeData.fillColor.alphaChannel);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] DestinationWidth = %d", curbeData.dstWidth);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] DestinationHeight = %d", curbeData.dstHeight);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] TotalNumberInputLayers = %d", curbeData.layerNum);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] FormatDescriptorDst = %d", curbeData.dstInfo.formatDescriptor);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] ChromaSittingLocationDst = %d", curbeData.dstInfo.chromaSitting);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] ChannelSwapEnablingFlagDst = %d", curbeData.dstInfo.channelSwap);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] DstCSCEnablingFlag = %d", curbeData.dstInfo.dstCscEnable);
    VP_RENDER_VERBOSEMESSAGE("[HDR Curbe] DitherRoundEnablingFlag = %d", curbeData.dstInfo.ditherRoundEnable);
}

}  // namespace vp
