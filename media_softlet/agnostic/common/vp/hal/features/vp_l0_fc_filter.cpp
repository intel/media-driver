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
//! \file     vp_l0_fc_filter.cpp
//! \brief    Defines the common interface for denoise
//!           this file is for the base interface which is shared by all l0 fc in driver.
//!
#include "vp_l0_fc_filter.h"
#include "vp_render_cmd_packet.h"
#include "igvpfc_common_args.h"
#include "igvpfc_fp_args.h"
#include <vector>

namespace vp
{

VpL0FcFilter::VpL0FcFilter(PVP_MHWINTERFACE vpMhwInterface) : VpFilter(vpMhwInterface)
{
}

MOS_STATUS VpL0FcFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_renderL0FcParams)
    {
        MOS_Delete(m_renderL0FcParams);
        m_renderL0FcParams = nullptr;
    }

    for (auto &handle : m_fcCommonKrnArgs)
    {
        KRN_ARG &krnArg = handle.second;
        MOS_FreeMemAndSetNull(krnArg.pData);
    }
    for (auto &handle : m_fcFastExpressKrnArgs)
    {
        KRN_ARG &krnArg = handle.second;
        MOS_FreeMemAndSetNull(krnArg.pData);
    }

    return MOS_STATUS_SUCCESS;
}

void L0_FC_KERNEL_PARAM::Init()
{
    kernelArgs.clear();
    kernelName.clear();
    kernelId     = kernelCombinedFc;
    threadWidth  = 0;
    threadHeight = 0;
    kernelConfig = {};
    kernelStatefulSurfaces.clear();
}

void _RENDER_L0_FC_PARAMS::Init()
{
    fc_kernelParams.clear();
}

MOS_STATUS VpL0FcFilter::SetExecuteEngineCaps(
    SwFilterPipe   *executingPipe,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executingPipe = executingPipe;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::InitKrnParams(L0_FC_KERNEL_PARAMS &krnParams, SwFilterPipe &executingPipe)
{
    VP_FUNC_CALL();

    krnParams.clear();

    L0_FC_COMP_PARAM compParam = {};
    VP_RENDER_CHK_STATUS_RETURN(InitCompParam(executingPipe, compParam));
    PrintCompParam(compParam);
    ReportDiffLog(compParam);
    
    L0_FC_KERNEL_PARAM param = {};
    if (FastExpressConditionMeet(compParam))
    {
        VP_RENDER_CHK_STATUS_RETURN(GenerateFcFastExpressKrnParam(compParam, param));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(GenerateFcCommonKrnParam(compParam, param));
    }
    //Set Perf Tag should be called after Generate Krn Param
    VP_PUBLIC_CHK_STATUS_RETURN(SetPerfTag(compParam, param.kernelConfig.perfTag));
    krnParams.push_back(param);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::GenerateFcCommonKrnParam(L0_FC_COMP_PARAM &compParam, L0_FC_KERNEL_PARAM &param)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);
    
    param = {};
    std::vector<L0_FC_KRN_IMAGE_PARAM> imageParams(compParam.layerNumber);
    L0_FC_KRN_TARGET_PARAM             targetParam = {};
    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        VP_RENDER_CHK_STATUS_RETURN(GenerateInputImageParam(compParam.inputLayersParam[i], compParam.mainCSpace, imageParams.at(i)));
    }
    VP_RENDER_CHK_STATUS_RETURN(GenerateTargetParam(compParam, targetParam));
    PrintKrnParam(imageParams, targetParam);

    uint32_t                     localSize[3]        = {128, 2, 1};  // localWidth, localHeight, localDepth
    uint32_t                     threadWidth         = targetParam.targetROI.right / localSize[0] + (targetParam.targetROI.right % localSize[0] != 0);
    uint32_t                     threadHeight        = targetParam.targetROI.bottom / localSize[1] + (targetParam.targetROI.bottom % localSize[1] != 0);
    KERNEL_ARGS                  krnArgs             = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces = {};

    auto handle = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find("FastComp_fc_common");
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = m_fcCommonKrnArgs.find(uIndex);
        if (argHandle == m_fcCommonKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = m_fcCommonKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_fcCommonKrnArgs);
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
    
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFcCommonKrnArg(compParam.layerNumber, imageParams, targetParam, localSize, krnArg, bInit));
    
        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }
    
    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t       uIndex       = kernelBti.first;
        SURFACE_PARAMS surfaceParam = {};
        bool           bInit        = true;
    
        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFcCommonBti(uIndex, compParam, surfaceParam, bInit));
    
        if (bInit)
        {
            krnStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
        }
    }
    
    param.kernelArgs             = krnArgs;
    param.kernelName             = "FastComp_fc_common";
    param.kernelId               = kernelL0FcCommon;
    param.threadWidth            = threadWidth;
    param.threadHeight           = threadHeight;
    param.localWidth             = localSize[0];
    param.localHeight            = localSize[1];
    param.kernelStatefulSurfaces = krnStatefulSurfaces;
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::SetupSingleFcCommonKrnArg(uint32_t layerNum, std::vector<L0_FC_KRN_IMAGE_PARAM> &imageParams, L0_FC_KRN_TARGET_PARAM &targetParam, uint32_t localSize[3], KRN_ARG &krnArg, bool &bInit)
{
    switch (krnArg.uIndex)
    {
    case FC_COMMON_FASTCOMP_LAYERNUMBER:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = layerNum;
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM0:
        if (imageParams.size() > 0)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(0)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(0), sizeof(imageParams.at(0)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM1:
        if (imageParams.size() > 1)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(1)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(1), sizeof(imageParams.at(1)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM2:
        if (imageParams.size() > 2)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(2)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(2), sizeof(imageParams.at(2)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM3:
        if (imageParams.size() > 3)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(3)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(3), sizeof(imageParams.at(3)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM4:
        if (imageParams.size() > 4)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(4)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(4), sizeof(imageParams.at(4)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM5:
        if (imageParams.size() > 5)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(5)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(5), sizeof(imageParams.at(5)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM6:
        if (imageParams.size() > 6)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(6)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(6), sizeof(imageParams.at(6)));
        }
        break;
    case FC_COMMON_FASTCOMP_IMAGEPARAM7:
        if (imageParams.size() > 7)
        {
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams.at(7)));
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams.at(7), sizeof(imageParams.at(7)));
        }
        break;

        break;
    case FC_COMMON_FASTCOMP_OUTPUTPARAM:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(targetParam));
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &targetParam, sizeof(targetParam));
        break;
    case FC_COMMON_FASTCOMP_INLINE_SAMPLER_LINEAR_CLAMP_EDGE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = MHW_SAMPLER_FILTER_BILINEAR;
        break;
    case FC_COMMON_FASTCOMP_INLINE_SAMPLER_NEAREST_CLAMP_EDGE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = MHW_SAMPLER_FILTER_NEAREST;
        break;
    case FC_COMMON_FASTCOMP_ENQUEUED_LOCAL_SIZE:
    case FC_COMMON_FASTCOMP_LOCAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = localSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = localSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = localSize[2];
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::SetupSingleFcCommonBti(uint32_t uIndex, const L0_FC_COMP_PARAM &compParam, SURFACE_PARAMS &surfaceParam, bool &bInit)
{
    switch (uIndex)
    {
    case FC_COMMON_FASTCOMP_INPUT0PL0:
        if (compParam.layerNumber > 0)
        {
            surfaceParam.surfType = SurfaceTypeFcInputLayer0;
            if (compParam.inputLayersParam[0].diParams.enabled &&
                compParam.inputLayersParam[0].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT1PL0:
        if (compParam.layerNumber > 1)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + 1);
            if (compParam.inputLayersParam[1].diParams.enabled &&
                compParam.inputLayersParam[1].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT2PL0:
        if (compParam.layerNumber > 2)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + 2);
            if (compParam.inputLayersParam[2].diParams.enabled &&
                compParam.inputLayersParam[2].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT3PL0:
        if (compParam.layerNumber > 3)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + 3);
            if (compParam.inputLayersParam[3].diParams.enabled &&
                compParam.inputLayersParam[3].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT4PL0:
        if (compParam.layerNumber > 4)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + 4);
            if (compParam.inputLayersParam[4].diParams.enabled &&
                compParam.inputLayersParam[4].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT5PL0:
        if (compParam.layerNumber > 5)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + 5);
            if (compParam.inputLayersParam[5].diParams.enabled &&
                compParam.inputLayersParam[5].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT6PL0:
        if (compParam.layerNumber > 6)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + 6);
            if (compParam.inputLayersParam[6].diParams.enabled &&
                compParam.inputLayersParam[6].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_INPUT7PL0:
        if (compParam.layerNumber > 7)
        {
            surfaceParam.surfType = SurfaceType(SurfaceTypeFcInputLayer0 + 7);
            if (compParam.inputLayersParam[7].diParams.enabled &&
                compParam.inputLayersParam[7].diParams.params.DIMode == DI_MODE_BOB)
            {
                surfaceParam.needVerticalStirde = true;
            }
        }
        else
        {
            surfaceParam.surfType = SurfaceTypeInvalid;
        }
        break;
    case FC_COMMON_FASTCOMP_OUTPUTPL0:
        surfaceParam.surfType = SurfaceTypeFcTarget0;
        surfaceParam.isOutput = true;
        break;
    case FC_COMMON_FASTCOMP_INPUT0PL1:
    case FC_COMMON_FASTCOMP_INPUT1PL1:
    case FC_COMMON_FASTCOMP_INPUT2PL1:
    case FC_COMMON_FASTCOMP_INPUT3PL1:
    case FC_COMMON_FASTCOMP_INPUT4PL1:
    case FC_COMMON_FASTCOMP_INPUT5PL1:
    case FC_COMMON_FASTCOMP_INPUT6PL1:
    case FC_COMMON_FASTCOMP_INPUT7PL1:
    case FC_COMMON_FASTCOMP_OUTPUTPL1:
        surfaceParam.surfType = SurfaceTypeInvalid;
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::InitCompParam(SwFilterPipe &executingPipe, L0_FC_COMP_PARAM &compParam)
{
    VP_FUNC_CALL();

    auto &surfGroup         = executingPipe.GetSurfacesSetting().surfGroup;
    compParam.layerNumber = executingPipe.GetSurfaceCount(true);
    if (SurfaceTypeFcInputLayer0 + compParam.layerNumber - 1 > SurfaceTypeFcInputLayerMax)
    {
        VP_RENDER_ASSERTMESSAGE("Invalid source count (%d)!", compParam.layerNumber);
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // Select default scaling mode for 3D sampler.
    VPHAL_SCALING_MODE defaultScalingMode = VPHAL_SCALING_NEAREST;
    VP_PUBLIC_CHK_STATUS_RETURN(GetDefaultScalingMode(defaultScalingMode, executingPipe));

    for (uint32_t i = 0; i < executingPipe.GetSurfaceCount(true); ++i)
    {
        VP_RENDER_CHK_STATUS_RETURN(InitLayer(executingPipe, true, i, defaultScalingMode, compParam.inputLayersParam[i]));
    }

    VP_RENDER_CHK_STATUS_RETURN(InitLayer(executingPipe, false, 0, defaultScalingMode, compParam.outputLayerParam));

    //use target Cspace as main color space instead of using RGB as main Cspace for no obvious difference
    compParam.mainCSpace = compParam.outputLayerParam.surf->ColorSpace;

    SwFilterColorFill *colorFill = dynamic_cast<SwFilterColorFill *>(executingPipe.GetSwFilter(false, 0, FeatureType::FeatureTypeColorFill));
    if (colorFill && colorFill->GetSwFilterParams().colorFillParams)
    {
        compParam.enableColorFill = true;
        compParam.colorFillParams = *colorFill->GetSwFilterParams().colorFillParams;
    }
    else
    {
        compParam.enableColorFill = false;
    }

    SwFilterAlpha *alpha = dynamic_cast<SwFilterAlpha *>(executingPipe.GetSwFilter(false, 0, FeatureType::FeatureTypeAlpha));
    if (alpha && alpha->GetSwFilterParams().compAlpha)
    {
        compParam.compAlpha             = *alpha->GetSwFilterParams().compAlpha;
        compParam.bAlphaCalculateEnable = alpha->GetSwFilterParams().calculatingAlpha;
    }
    else
    {
        compParam.bAlphaCalculateEnable = false;
        compParam.compAlpha               = {};
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::InitLayer(SwFilterPipe& executingPipe, bool isInputPipe, int index, VPHAL_SCALING_MODE defaultScalingMode, L0_FC_LAYER_PARAM& layer)
{
    VP_FUNC_CALL();
    auto &surfGroup = executingPipe.GetSurfacesSetting().surfGroup;

    SurfaceType surfId     = isInputPipe ? (SurfaceType)(SurfaceTypeFcInputLayer0 + index) : SurfaceTypeFcTarget0;
    auto        surfHandle = surfGroup.find(surfId);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(surfHandle, &surfGroup);
    layer.surf = surfHandle->second;

    VP_PUBLIC_CHK_NULL_RETURN(layer.surf);

    layer.layerID       = index;
    layer.layerIDOrigin = index;

    SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeScaling));
    layer.scalingMode        = scaling ? scaling->GetSwFilterParams().scalingMode : defaultScalingMode;

    SwFilterRotMir *rotation = dynamic_cast<SwFilterRotMir *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeRotMir));
    layer.rotation           = rotation ? rotation->GetSwFilterParams().rotation : VPHAL_ROTATION_IDENTITY;

    SwFilterDeinterlace *di = dynamic_cast<SwFilterDeinterlace *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeDi));
    if (di && di->GetSwFilterParams().diParams)
    {
        layer.diParams.enabled = true;
        layer.diParams.params  = *di->GetSwFilterParams().diParams;
    }
    else
    {
        layer.diParams.enabled = false;
    }

    SwFilterLumakey *lumakey = dynamic_cast<SwFilterLumakey *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeLumakey));
    if (lumakey && lumakey->GetSwFilterParams().lumaKeyParams)
    {
        layer.lumaKey.enabled = true;
        layer.lumaKey.params  = *lumakey->GetSwFilterParams().lumaKeyParams;
    }
    else
    {
        layer.lumaKey.enabled = false;
    }

    SwFilterBlending *blending = dynamic_cast<SwFilterBlending *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeBlending));
    if (blending && blending->GetSwFilterParams().blendingParams)
    {
        layer.blendingParams = *blending->GetSwFilterParams().blendingParams;
    }
    else
    {
        layer.blendingParams.BlendType = BLEND_NONE;
    }

    //procamp is not enabled for L0 FC
    SwFilterProcamp *procamp = dynamic_cast<SwFilterProcamp *>(executingPipe.GetSwFilter(isInputPipe, index, FeatureType::FeatureTypeProcamp));
    if (procamp && procamp->GetSwFilterParams().procampParams)
    {
        layer.procampParams = *procamp->GetSwFilterParams().procampParams;
    }
    else
    {
        layer.procampParams.bEnabled = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::GetDefaultScalingMode(VPHAL_SCALING_MODE &defaultScalingMode, SwFilterPipe &executedPipe)
{
    bool isInited = false;
    // Select default scaling mode for 3D sampler.
    defaultScalingMode = VPHAL_SCALING_NEAREST;

    if (!PolicyFcHandler::s_forceNearestToBilinearIfBilinearExists)
    {
        return MOS_STATUS_SUCCESS;
    }

    for (uint32_t i = 0; i < executedPipe.GetSurfaceCount(true); ++i)
    {
        SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(executedPipe.GetSwFilter(true, i, FeatureType::FeatureTypeScaling));
        if (scaling &&
            (VPHAL_SCALING_NEAREST == scaling->GetSwFilterParams().scalingMode ||
                VPHAL_SCALING_BILINEAR == scaling->GetSwFilterParams().scalingMode))
        {
            if (isInited)
            {
                if (scaling->GetSwFilterParams().scalingMode != defaultScalingMode)
                {
                    VP_PUBLIC_ASSERTMESSAGE("Different 3D sampler scaling mode being selected!");
                    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
                }
            }
            else
            {
                defaultScalingMode = scaling->GetSwFilterParams().scalingMode;
                isInited           = true;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::GetChromaSitingFactor(MOS_FORMAT format, uint8_t& hitSecPlaneFactorX, uint8_t& hitSecPlaneFactorY)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A16R16G16B16:
    case Format_R10G10B10A2:
    case Format_AYUV:
    case Format_A16R16G16B16F:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_A16B16G16R16:
    case Format_B10G10R10A2:
    case Format_A16B16G16R16F:
    case Format_Y410:
    case Format_Y416:
    case Format_400P:
        hitSecPlaneFactorX = 1;
        hitSecPlaneFactorY = 1;
        break;
    case Format_NV12:
    case Format_P010:
    case Format_P016:
    case Format_P210:
    case Format_P216:
        hitSecPlaneFactorX = 2;
        hitSecPlaneFactorY = 2;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_Y210:
    case Format_Y216:
        hitSecPlaneFactorX = 2;
        hitSecPlaneFactorY = 1;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::GetBitNumber(MOS_FORMAT format, uint8_t *pOriginBitNumber, uint8_t *pStoredBitNumber, uint8_t *pAlphaBitNumber)
{
    uint8_t storedBitNumber = 0;
    uint8_t originBitNumber = 0;
    uint8_t alphaBitNumber  = 0;

    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_AYUV:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
        storedBitNumber = 8;
        originBitNumber = 8;
        alphaBitNumber  = 8;
        break;
    case Format_A16R16G16B16:
    case Format_A16B16G16R16:
    case Format_Y416:
        storedBitNumber = 16;
        originBitNumber = 16;
        alphaBitNumber  = 16;
        break;
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
    case Format_Y410:
        storedBitNumber = 10;
        originBitNumber = 10;
        alphaBitNumber  = 2;
        break;
    case Format_A16R16G16B16F:
    case Format_A16B16G16R16F:
        storedBitNumber = 0;
        originBitNumber = 0;
        alphaBitNumber  = 0;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_NV12:
    case Format_400P:
        storedBitNumber = 8;
        originBitNumber = 8;
        alphaBitNumber  = 0;
        break;
    case Format_Y210:
    case Format_P010:
    case Format_P210:
        storedBitNumber = 16;
        originBitNumber = 10;
        alphaBitNumber  = 0;
        break;
    case Format_Y216:
    case Format_P016:
    case Format_P216:
        storedBitNumber = 16;
        originBitNumber = 16;
        alphaBitNumber  = 0;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    }

    if (pOriginBitNumber)
    {
        *pOriginBitNumber = originBitNumber;
    }
    if (pStoredBitNumber)
    {
        *pStoredBitNumber = storedBitNumber;
    }
    if (pAlphaBitNumber)
    {
        *pAlphaBitNumber = alphaBitNumber;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::GenerateInputImageParam(L0_FC_LAYER_PARAM &layer, VPHAL_CSPACE mainCSpace, L0_FC_KRN_IMAGE_PARAM &imageParam)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(layer.surf);
    VP_PUBLIC_CHK_NULL_RETURN(layer.surf->osSurface);

    uint32_t inputWidth  = MOS_MIN(static_cast<uint32_t>(layer.surf->osSurface->dwWidth), static_cast<uint32_t>(layer.surf->rcSrc.right));
    uint32_t inputHeight = MOS_MIN(static_cast<uint32_t>(layer.surf->osSurface->dwHeight), static_cast<uint32_t>(layer.surf->rcSrc.bottom));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertCscToKrnParam(layer.surf->ColorSpace, mainCSpace, imageParam.csc));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputChannelIndicesToKrnParam(layer.surf->osSurface->Format, imageParam.inputChannelIndices));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertScalingRotToKrnParam(layer.surf->rcSrc, layer.surf->rcDst, layer.scalingMode, inputWidth, inputHeight, layer.rotation, imageParam.scale, imageParam.controlSetting.samplerType, imageParam.coordShift));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertChromaUpsampleToKrnParam(layer.surf->osSurface->Format, layer.surf->ChromaSiting, layer.scalingMode, inputWidth, inputHeight, imageParam.coordShift.chromaShiftX, imageParam.coordShift.chromaShiftY, imageParam.controlSetting.isChromaShift));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertPlaneNumToKrnParam(layer.surf->osSurface->Format, true, imageParam.inputPlaneNum));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertBlendingToKrnParam(layer.blendingParams, imageParam.controlSetting.ignoreSrcPixelAlpha, imageParam.controlSetting.ignoreDstPixelAlpha, imageParam.constAlphs));    
    
    if (layer.lumaKey.enabled)
    {
        imageParam.lumaKey.low  = (float)layer.lumaKey.params.LumaLow / 255;
        imageParam.lumaKey.high = (float)layer.lumaKey.params.LumaHigh / 255;
    }
    else
    {
        imageParam.lumaKey.low  = -1.f;
        imageParam.lumaKey.high = -1.f;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertRotationToKrnParam(VPHAL_ROTATION rotation, float strideX, float strideY, float startLeft, float startRight, float startTop, float startBottom, L0_FC_KRN_SCALE_PARAM &scaling)
{
    switch (rotation)
    {
    case VPHAL_ROTATION_IDENTITY:
        scaling.rotateIndices[0]  = 0;
        scaling.rotateIndices[1]  = 1;
        scaling.src.startX        = startLeft;
        scaling.src.startY        = startTop;
        scaling.src.strideX       = strideX;
        scaling.src.strideY       = strideY;
        break;
    case VPHAL_ROTATION_90:
        scaling.rotateIndices[0]  = 1;
        scaling.rotateIndices[1]  = 0;
        scaling.src.startX        = startLeft;
        scaling.src.startY        = startBottom;
        scaling.src.strideX       = strideY;
        scaling.src.strideY       = -strideX;
        break;
    case VPHAL_ROTATION_180:
        scaling.rotateIndices[0]  = 0;
        scaling.rotateIndices[1]  = 1;
        scaling.src.startX        = startRight;
        scaling.src.startY        = startBottom;
        scaling.src.strideX       = -strideX;
        scaling.src.strideY       = -strideY;
        break;
    case VPHAL_ROTATION_270:
        scaling.rotateIndices[0]  = 1;
        scaling.rotateIndices[1]  = 0;
        scaling.src.startX        = startRight;
        scaling.src.startY        = startTop;
        scaling.src.strideX       = -strideY;
        scaling.src.strideY       = strideX;
        break;
    case VPHAL_MIRROR_HORIZONTAL:
        scaling.rotateIndices[0]  = 0;
        scaling.rotateIndices[1]  = 1;
        scaling.src.startX        = startRight;
        scaling.src.startY        = startTop;
        scaling.src.strideX       = -strideX;
        scaling.src.strideY       = strideY;
        break;
    case VPHAL_MIRROR_VERTICAL:
        scaling.rotateIndices[0]  = 0;
        scaling.rotateIndices[1]  = 1;
        scaling.src.startX        = startLeft;
        scaling.src.startY        = startBottom;
        scaling.src.strideX       = strideX;
        scaling.src.strideY       = -strideY;
        break;
    case VPHAL_ROTATE_90_MIRROR_VERTICAL:
        scaling.rotateIndices[0]  = 1;
        scaling.rotateIndices[1]  = 0;
        scaling.src.startX        = startRight;
        scaling.src.startY        = startBottom;
        scaling.src.strideX       = -strideY;
        scaling.src.strideY       = -strideX;
        break;
    case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
        scaling.rotateIndices[0]  = 1;
        scaling.rotateIndices[1]  = 0;
        scaling.src.startX        = startLeft;
        scaling.src.startY        = startTop;
        scaling.src.strideX       = strideY;
        scaling.src.strideY       = strideX;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertCscToKrnParam(VPHAL_CSPACE srcColorSpace, VPHAL_CSPACE dstColorSpace, L0_FC_KRN_CSC_MATRIX &csc)
{
    VP_FUNC_CALL();
    csc = {};
    if (srcColorSpace == dstColorSpace)
    {
        csc.s0123[0] = csc.s4567[1] = csc.s89AB[2] = 1;
        return MOS_STATUS_SUCCESS;
    }
    
    float cscMatrix[12] = {};
    KernelDll_GetCSCMatrix(srcColorSpace, dstColorSpace, cscMatrix);
    VP_PUBLIC_CHK_STATUS_RETURN(MOS_SecureMemcpy(csc.s0123, sizeof(csc.s0123), &cscMatrix[0], sizeof(float) * 3));
    VP_PUBLIC_CHK_STATUS_RETURN(MOS_SecureMemcpy(csc.s4567, sizeof(csc.s4567), &cscMatrix[4], sizeof(float) * 3));
    VP_PUBLIC_CHK_STATUS_RETURN(MOS_SecureMemcpy(csc.s89AB, sizeof(csc.s89AB), &cscMatrix[8], sizeof(float) * 3));
    csc.sCDEF[0] = cscMatrix[3] / 255;
    csc.sCDEF[1] = cscMatrix[7] / 255;
    csc.sCDEF[2] = cscMatrix[11] / 255;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertScalingRotToKrnParam(
    RECT &rcSrc, 
    RECT &rcDst, 
    VPHAL_SCALING_MODE scalingMode, 
    uint32_t inputWidth, 
    uint32_t inputHeight, 
    VPHAL_ROTATION rotation, 
    L0_FC_KRN_SCALE_PARAM &scaling, 
    uint8_t &samplerType, 
    L0_FC_KRN_COORD_SHIFT_PARAM &coordShift)

{
    VP_FUNC_CALL();
    if (scalingMode == VPHAL_SCALING_BILINEAR)
    {
        coordShift.commonShiftX  = VP_HW_LINEAR_SHIFT / inputWidth;
        coordShift.commonShiftY = VP_HW_LINEAR_SHIFT / inputHeight;
        samplerType             = 1;
    }
    else if (scalingMode == VPHAL_SCALING_NEAREST)
    {
        coordShift.commonShiftX  = VP_SAMPLER_BIAS / inputWidth;
        coordShift.commonShiftY  = VP_SAMPLER_BIAS / inputHeight;
        samplerType             = 0;
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    scaling.trg.left   = rcDst.left;
    scaling.trg.right  = rcDst.right;
    scaling.trg.top    = rcDst.top;
    scaling.trg.bottom = rcDst.bottom;

    float strideX     = (float)(rcSrc.right - rcSrc.left) / (rcDst.right - rcDst.left) / inputWidth;
    float strideY     = (float)(rcSrc.bottom - rcSrc.top) / (rcDst.bottom - rcDst.top) / inputHeight;
    float startLeft   = (float)rcSrc.left / inputWidth;
    float startRight  = (float)(rcSrc.right - 1) / inputWidth;
    float startTop    = (float)rcSrc.top / inputHeight;
    float startBottom = (float)(rcSrc.bottom - 1) / inputHeight;
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertRotationToKrnParam(rotation, strideX, strideY, startLeft, startRight, startTop, startBottom, scaling));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertChromaUpsampleToKrnParam(MOS_FORMAT format, uint32_t chromaSitingLoc, VPHAL_SCALING_MODE scalingMode, uint32_t inputWidth, uint32_t inputHeight, float &chromaShiftX, float &chromaShiftY, uint8_t &isChromaShift)
{
    uint8_t hitSecPlaneFactorX = 0;
    uint8_t hitSecPlaneFactorY = 0;
    isChromaShift              = 0;
    chromaShiftX               = 0.5f;
    chromaShiftY               = 0.5f;
    VP_PUBLIC_CHK_STATUS_RETURN(GetChromaSitingFactor(format, hitSecPlaneFactorX, hitSecPlaneFactorY));    

    // If there is no DDI setting, we use the Horizontal Left Vertical Center as default for PL2 surface.
    if (chromaSitingLoc == CHROMA_SITING_NONE)
    {
        // PL2 default to Horizontal Left, Vertical Center
        if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 2)
        {
            //Center Left
            isChromaShift = 1;
            chromaShiftY -= 0.5f;
        }
    }
    else
    {
        // PL2, 6 positions are available
        if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 2)
        {
            // Horizontal Left
            if (chromaSitingLoc & CHROMA_SITING_HORZ_LEFT)
            {
                if (chromaSitingLoc & CHROMA_SITING_VERT_TOP)
                {
                    //Top Left
                    isChromaShift = 1;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_CENTER)
                {
                    //Center Left
                    isChromaShift = 1;
                    chromaShiftY -= 0.5f;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_BOTTOM)
                {
                    //Bottom Left
                    isChromaShift = 1;
                    chromaShiftY -= 1.f;
                }
            }
            // Horizontal Center
            else if (chromaSitingLoc & CHROMA_SITING_HORZ_CENTER)
            {
                chromaShiftX -= 0.5f;
                if (chromaSitingLoc & CHROMA_SITING_VERT_TOP)
                {
                    //Top Center
                    isChromaShift = 1;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_CENTER)
                {
                    //Center Center
                    isChromaShift = 0;
                    chromaShiftY -= 0.5f;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_BOTTOM)
                {
                    //Bottom Center
                    isChromaShift = 1;
                    chromaShiftY -= 1.f;
                }
            }
        }
        else if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 1)
        {
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (chromaSitingLoc & (CHROMA_SITING_HORZ_CENTER))
            {
                //Top Center
                isChromaShift = 1;
                chromaShiftX -= 0.5f;
            }
        }
    }

    if (scalingMode == VPHAL_SCALING_NEAREST)
    {
        isChromaShift = 0;
    }
    chromaShiftX /= inputWidth;
    chromaShiftY /= inputHeight;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertInputChannelIndicesToKrnParam(MOS_FORMAT format, uint32_t* inputChannelIndices)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_B10G10R10A2:
    case Format_A16R16G16B16:
    case Format_A16R16G16B16F:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 1;
        inputChannelIndices[2] = 2;
        inputChannelIndices[3] = 3;
        break;
    case Format_AYUV:
        inputChannelIndices[0] = 1;
        inputChannelIndices[1] = 2;
        inputChannelIndices[2] = 0;
        inputChannelIndices[3] = 3;
        break;
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_R10G10B10A2:
    case Format_A16B16G16R16:
    case Format_A16B16G16R16F:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 1;
        inputChannelIndices[2] = 2;
        inputChannelIndices[3] = 3;
        break;
    case Format_YUY2:
    case Format_YUYV: 
    case Format_Y210:
    case Format_Y216:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 5;
        inputChannelIndices[2] = 7;
        inputChannelIndices[3] = 3;
        break;
    case Format_YVYU:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 7;
        inputChannelIndices[2] = 5;
        inputChannelIndices[3] = 3;
        break;
    case Format_UYVY:
        inputChannelIndices[0] = 1;
        inputChannelIndices[1] = 4;
        inputChannelIndices[2] = 6;
        inputChannelIndices[3] = 3;
        break;
    case Format_VYUY:
        inputChannelIndices[0] = 1;
        inputChannelIndices[1] = 6;
        inputChannelIndices[2] = 4;
        inputChannelIndices[3] = 3;
        break;
    case Format_Y410:
    case Format_Y416:
        inputChannelIndices[0] = 1;
        inputChannelIndices[1] = 0;
        inputChannelIndices[2] = 2;
        inputChannelIndices[3] = 3;
        break;
    case Format_NV12:
    case Format_P010:
    case Format_P016:
    case Format_P210:
    case Format_P216:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 4;
        inputChannelIndices[2] = 5;
        inputChannelIndices[3] = 3;
        break;
    case Format_400P:
        inputChannelIndices[0] = 0;
        inputChannelIndices[1] = 0;
        inputChannelIndices[2] = 0;
        inputChannelIndices[3] = 3;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertPlaneNumToKrnParam(MOS_FORMAT format, bool isInput, uint32_t &planeNum)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A16R16G16B16:
    case Format_R10G10B10A2:
    case Format_AYUV:
    case Format_A16R16G16B16F:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_A16B16G16R16:
    case Format_B10G10R10A2:
    case Format_A16B16G16R16F:
    case Format_Y410:
    case Format_Y416:
    case Format_400P:
        planeNum = 1;
        break;
    case Format_NV12:
    case Format_P010:
    case Format_P016:
    case Format_P210:
    case Format_P216:
        planeNum = 2;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_Y210:
    case Format_Y216:
        if (isInput)
        {
            planeNum = 2; 
        }
        else
        {
            planeNum = 3;
        }
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertBlendingToKrnParam(VPHAL_BLENDING_PARAMS &blend, uint8_t &ignoreSrcPixelAlpha, uint8_t &ignoreDstPixelAlpha, float &constAlpha)
{
    switch (blend.BlendType)
    {
    case BLEND_NONE:
        ignoreSrcPixelAlpha = 1;
        constAlpha          = 1;
        ignoreDstPixelAlpha = 1;
        break;
    case BLEND_SOURCE:
        ignoreSrcPixelAlpha = 0;
        constAlpha          = 1;
        ignoreDstPixelAlpha = 0;
        break;
    case BLEND_PARTIAL:
        ignoreSrcPixelAlpha = 1;
        constAlpha          = 1;
        ignoreDstPixelAlpha = 0;
        break;
    case BLEND_CONSTANT:
        ignoreSrcPixelAlpha = 1;
        constAlpha          = blend.fAlpha;
        ignoreDstPixelAlpha = 1;
        break;
    case BLEND_CONSTANT_SOURCE:
        ignoreSrcPixelAlpha = 0;
        constAlpha          = blend.fAlpha;
        ignoreDstPixelAlpha = 0;
        break;
    case BLEND_CONSTANT_PARTIAL:
        ignoreSrcPixelAlpha = 1;
        constAlpha          = blend.fAlpha;
        ignoreDstPixelAlpha = 0;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::GenerateTargetParam(L0_FC_COMP_PARAM &compParam, L0_FC_KRN_TARGET_PARAM &targetParam)
{
    VP_FUNC_CALL();
    VP_SURFACE *targetSurf = compParam.outputLayerParam.surf;
    VP_PUBLIC_CHK_NULL_RETURN(targetSurf);
    VP_PUBLIC_CHK_NULL_RETURN(targetSurf->osSurface);
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertPlaneNumToKrnParam(targetSurf->osSurface->Format, false, targetParam.planeNumber));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertOuputChannelIndicesToKrnParam(targetSurf->osSurface->Format, targetParam.dynamicChannelIndices));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertTargetRoiToKrnParam(targetSurf->rcDst, targetSurf->osSurface->dwWidth, targetSurf->osSurface->dwHeight, targetParam.targetROI));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertChromaDownsampleToKrnParam(targetSurf->osSurface->Format, targetSurf->ChromaSiting, targetParam.chromaSitingFactor, targetParam.controlSetting.hitSecPlaneFactorX, targetParam.controlSetting.hitSecPlaneFactorY));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertColorFillToKrnParam(compParam.enableColorFill, compParam.colorFillParams, compParam.mainCSpace, targetParam.controlSetting.isColorFill, targetParam.background));
    //ConvertAlphaToKrnParam must be called after ConvertColorFillToKrnParam
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertAlphaToKrnParam(compParam.bAlphaCalculateEnable, compParam.compAlpha, targetParam.background[3], targetParam.controlSetting.alphaLayerIndex, targetParam.alpha));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertOuputChannelIndicesToKrnParam(MOS_FORMAT format, uint32_t *dynamicChannelIndices)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_B10G10R10A2:
    case Format_A16R16G16B16:
    case Format_A16R16G16B16F:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 1;
        dynamicChannelIndices[2] = 2;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_AYUV:
        dynamicChannelIndices[0] = 2;
        dynamicChannelIndices[1] = 0;
        dynamicChannelIndices[2] = 1;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_R10G10B10A2:
    case Format_A16B16G16R16:
    case Format_A16B16G16R16F:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 1;
        dynamicChannelIndices[2] = 2;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_Y410:
    case Format_Y416:
        dynamicChannelIndices[0] = 1;
        dynamicChannelIndices[1] = 0;
        dynamicChannelIndices[2] = 2;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_NV12:
    case Format_P010:
    case Format_P016:
    case Format_P210:
    case Format_P216:
        dynamicChannelIndices[0] = 1;
        dynamicChannelIndices[1] = 2;
        dynamicChannelIndices[2] = 3;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_400P:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 0;
        dynamicChannelIndices[2] = 0;
        dynamicChannelIndices[3] = 3;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_Y210:
    case Format_Y216:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 1;
        dynamicChannelIndices[2] = 3;
        break;
    case Format_YVYU:
        dynamicChannelIndices[0] = 0;
        dynamicChannelIndices[1] = 3;
        dynamicChannelIndices[2] = 1;
        break;
    case Format_UYVY:
        dynamicChannelIndices[0] = 1;
        dynamicChannelIndices[1] = 0;
        dynamicChannelIndices[2] = 2;
        break;
    case Format_VYUY:
        dynamicChannelIndices[0] = 1;
        dynamicChannelIndices[1] = 2;
        dynamicChannelIndices[2] = 0;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertTargetRoiToKrnParam(RECT &outputRcDst, uint32_t outputWidth, uint32_t outputHeight, L0_FC_KRN_RECT &targetROI)
{
    
    targetROI.left   = MOS_MAX(0, outputRcDst.left);
    targetROI.right  = MOS_MIN((uint64_t)outputRcDst.right, outputWidth);
    targetROI.top    = MOS_MAX(0, outputRcDst.top);
    targetROI.bottom = MOS_MIN((uint64_t)outputRcDst.bottom, outputHeight);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertChromaDownsampleToKrnParam(MOS_FORMAT format, uint32_t chromaSitingLoc, float *chromaSitingFactor, uint8_t &hitSecPlaneFactorX, uint8_t &hitSecPlaneFactorY)
{
    VP_PUBLIC_CHK_STATUS_RETURN(GetChromaSitingFactor(format, hitSecPlaneFactorX, hitSecPlaneFactorY));

    //Left Top
    chromaSitingFactor[0] = 1;
    chromaSitingFactor[1] = 0;
    chromaSitingFactor[2] = 0;
    chromaSitingFactor[3] = 0;

    if (chromaSitingLoc== CHROMA_SITING_NONE)
    {
        // PL2 default to Horizontal Left, Vertical Center
        if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 2)
        {
            //Center Left
            chromaSitingFactor[0] = 0.5;
            chromaSitingFactor[1] = 0;
            chromaSitingFactor[2] = 0.5;
            chromaSitingFactor[3] = 0;
        }
    }
    else
    {
        // PL2, 6 positions are avalibale
        if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 2)
        {
            // Horizontal Left
            if (chromaSitingLoc & CHROMA_SITING_HORZ_LEFT)
            {
                if (chromaSitingLoc & CHROMA_SITING_VERT_TOP)
                {
                    //Top Left
                    chromaSitingFactor[0] = 1;
                    chromaSitingFactor[1] = 0;
                    chromaSitingFactor[2] = 0;
                    chromaSitingFactor[3] = 0;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_CENTER)
                {
                    //Center Left
                    chromaSitingFactor[0] = 0.5;
                    chromaSitingFactor[1] = 0;
                    chromaSitingFactor[2] = 0.5;
                    chromaSitingFactor[3] = 0;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_BOTTOM)
                {
                    //Bottom Left
                    chromaSitingFactor[0] = 0;
                    chromaSitingFactor[1] = 0;
                    chromaSitingFactor[2] = 1;
                    chromaSitingFactor[3] = 0;
                }
            }
            // Horizontal Center
            else if (chromaSitingLoc & CHROMA_SITING_HORZ_CENTER)
            {
                if (chromaSitingLoc & CHROMA_SITING_VERT_TOP)
                {
                    //Top Center
                    chromaSitingFactor[0] = 0.5;
                    chromaSitingFactor[1] = 0.5;
                    chromaSitingFactor[2] = 0;
                    chromaSitingFactor[3] = 0;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_CENTER)
                {
                    //Center Center
                    chromaSitingFactor[0] = 0.25;
                    chromaSitingFactor[1] = 0.25;
                    chromaSitingFactor[2] = 0.25;
                    chromaSitingFactor[3] = 0.25;
                }
                else if (chromaSitingLoc & CHROMA_SITING_VERT_BOTTOM)
                {
                    //Bottom Center
                    chromaSitingFactor[0] = 0;
                    chromaSitingFactor[1] = 0;
                    chromaSitingFactor[2] = 0.5;
                    chromaSitingFactor[3] = 0.5;
                }
            }
        }
        else if (hitSecPlaneFactorX == 2 && hitSecPlaneFactorY == 1)
        {
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (chromaSitingLoc & CHROMA_SITING_HORZ_CENTER)
            {
                //Top Center
                chromaSitingFactor[0] = 0.5;
                chromaSitingFactor[1] = 0.5;
                chromaSitingFactor[2] = 0;
                chromaSitingFactor[3] = 0;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertAlphaToKrnParam(bool bAlphaCalculateEnable, VPHAL_ALPHA_PARAMS &compAlpha, float colorFillAlpha, uint8_t &alphaLayerIndex, float &alpha)
{
    switch (compAlpha.AlphaMode)
    {
    case VPHAL_ALPHA_FILL_MODE_NONE:
        alpha           = compAlpha.fAlpha;
        alphaLayerIndex = 8;
        break;
    case VPHAL_ALPHA_FILL_MODE_OPAQUE:
        alpha           = 1.f;
        alphaLayerIndex = 8;
        break;
    case VPHAL_ALPHA_FILL_MODE_BACKGROUND:
        alpha           = colorFillAlpha;
        alphaLayerIndex = 8;
        break;
    case VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM:
        alphaLayerIndex = 0;
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    if (!bAlphaCalculateEnable)
    {
        alpha           = 0;
        alphaLayerIndex = 8;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertColorFillToKrnParam(bool enableColorFill, VPHAL_COLORFILL_PARAMS &colorFillParams, MEDIA_CSPACE dstCspace, uint8_t &isColorFill, float *background)
{
    VP_FUNC_CALL();
    isColorFill = enableColorFill;
    if (enableColorFill)
    {
        MEDIA_CSPACE         srcCspace = colorFillParams.CSpace;
        VPHAL_COLOR_SAMPLE_8 srcColor  = {};
        VPHAL_COLOR_SAMPLE_8 dstColor  = {};
        srcColor.dwValue               = colorFillParams.Color;
        if (!VpUtils::GetCscMatrixForRender8Bit(&dstColor, &srcColor, srcCspace, dstCspace))
        {
            VP_PUBLIC_ASSERTMESSAGE("Color fill covert fail. srcCspace %d, dstCspace %d", srcCspace, dstCspace);
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("Color fill background covert from srcCspace %d to dstCspace %d", srcCspace, dstCspace);
        }

        if ((dstCspace == CSpace_sRGB) || (dstCspace == CSpace_stRGB) || IS_COLOR_SPACE_BT2020_RGB(dstCspace))
        {
            background[0] = (float)dstColor.R / 255;
            background[1] = (float)dstColor.G / 255;
            background[2] = (float)dstColor.B / 255;
            background[3] = (float)dstColor.A / 255;
        }
        else
        {
            background[0] = (float)dstColor.Y / 255;
            background[1] = (float)dstColor.U / 255;
            background[2] = (float)dstColor.V / 255;
            background[3] = (float)dstColor.a / 255;
        }
    }

    return MOS_STATUS_SUCCESS;
}
bool VpL0FcFilter::FastExpressConditionMeet(const L0_FC_COMP_PARAM &compParam)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_pvpMhwInterface &&
        m_pvpMhwInterface->m_userFeatureControl &&
        m_pvpMhwInterface->m_userFeatureControl->DisableL0FcFp())
    {
        return false;
    }
#endif
    if (compParam.layerNumber != 1)
    {
        return false;
    }
    const L0_FC_LAYER_PARAM &inputLayer  = compParam.inputLayersParam[0];
    const L0_FC_LAYER_PARAM &outputLayer = compParam.outputLayerParam;
    VP_SURFACE              *inputSurf   = inputLayer.surf;
    VP_SURFACE              *outputSurf  = outputLayer.surf;
    if (!outputSurf ||
        !outputSurf->osSurface ||
        !inputSurf ||
        !inputSurf->osSurface)
    {
        return false;
    }
    bool trgFormatSupport = outputSurf->osSurface->Format == Format_NV12 ||
                            outputSurf->osSurface->Format == Format_P010 ||
                            outputSurf->osSurface->Format == Format_P016;
    bool isAligned = false;
    if (compParam.enableColorFill)
    {
        isAligned = MOS_IS_ALIGNED(MOS_MAX(0, outputSurf->rcDst.left), 2) &&
                    MOS_IS_ALIGNED(MOS_MIN(outputSurf->osSurface->dwWidth, (uint64_t)outputSurf->rcDst.right), 2) &&
                    MOS_IS_ALIGNED(MOS_MAX(0, outputSurf->rcDst.top), 2) &&
                    MOS_IS_ALIGNED(MOS_MIN(outputSurf->osSurface->dwHeight, (uint64_t)outputSurf->rcDst.bottom), 2);
    }
    else
    {
        isAligned = MOS_IS_ALIGNED(MOS_MAX(0, inputSurf->rcDst.left), 2) &&
                    MOS_IS_ALIGNED(MOS_MIN(outputSurf->osSurface->dwWidth, (uint64_t)inputSurf->rcDst.right), 2) &&
                    MOS_IS_ALIGNED(MOS_MAX(0, inputSurf->rcDst.top), 2) &&
                    MOS_IS_ALIGNED(MOS_MIN(outputSurf->osSurface->dwHeight, (uint64_t)inputSurf->rcDst.bottom), 2);
    }
    if (!trgFormatSupport                                 ||
        !isAligned                                        ||
        inputLayer.blendingParams.BlendType != BLEND_NONE ||
        inputLayer.diParams.enabled                       ||
        inputLayer.lumaKey.enabled)
    {
        return false;
    }

    return true;
}

MOS_STATUS VpL0FcFilter::GenerateFcFastExpressKrnParam(L0_FC_COMP_PARAM &compParam, L0_FC_KERNEL_PARAM &param)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);

    param = {};
    L0_FC_FP_KRN_IMAGE_PARAM  imageParam  = {};
    L0_FC_FP_KRN_TARGET_PARAM targetParam = {};
    VP_RENDER_CHK_STATUS_RETURN(GenerateFastExpressInputOutputParam(compParam, imageParam, targetParam));

    uint32_t                     localSize[3]        = {16, 2, 1};  // localWidth, localHeight, localDepth
    uint32_t                     workItemNum         = targetParam.alignedTrgRectSize.width * targetParam.alignedTrgRectSize.height;
    uint32_t                     localWorkItemNum    = localSize[0] * localSize[1];
    uint32_t                     groupNumber         = workItemNum / localWorkItemNum + (workItemNum % localWorkItemNum != 0);
    uint32_t                     threadGroupWidth    = 8;
    uint32_t                     threadGroupHeight   = groupNumber / threadGroupWidth + (groupNumber % threadGroupWidth != 0);
    uint32_t                     globalSize[3]       = {localSize[0] * threadGroupWidth, localSize[1] * threadGroupHeight, 1};
    KERNEL_ARGS                  krnArgs             = {};
    KERNEL_ARG_INDEX_SURFACE_MAP krnStatefulSurfaces = {};

    auto handle = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find("FastExpress_fc_fp"); 
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = m_fcFastExpressKrnArgs.find(uIndex);
        if (argHandle == m_fcFastExpressKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = m_fcFastExpressKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_fcFastExpressKrnArgs);
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

        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFcFastExpressKrnArg(imageParam, targetParam, localSize, globalSize, krnArg, bInit));

        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t       uIndex       = kernelBti.first;
        SURFACE_PARAMS surfaceParam = {};
        bool           bInit        = true;

        VP_PUBLIC_CHK_STATUS_RETURN(SetupSingleFcFastExpressBti(uIndex, compParam, surfaceParam, bInit));

        if (bInit)
        {
            krnStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
        }
    }

    param.kernelArgs             = krnArgs;
    param.kernelName             = "FastExpress_fc_fp";
    param.kernelId               = kernelL0FcFP;
    param.threadWidth            = threadGroupWidth;
    param.threadHeight           = threadGroupHeight;
    param.localWidth             = localSize[0];
    param.localHeight            = localSize[1];
    param.kernelStatefulSurfaces = krnStatefulSurfaces;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::SetupSingleFcFastExpressBti(uint32_t uIndex, const L0_FC_COMP_PARAM &compParam, SURFACE_PARAMS &surfaceParam, bool &bInit)
{
    switch (uIndex)
    {
    case FC_FP_FASTEXPRESS_INPUTPL0:
        surfaceParam.surfType = SurfaceTypeFcInputLayer0;
        if (compParam.inputLayersParam[0].diParams.enabled &&
            compParam.inputLayersParam[0].diParams.params.DIMode == DI_MODE_BOB)
        {
            surfaceParam.needVerticalStirde = true;
        }
        break;
    case FC_FP_FASTEXPRESS_OUTPUTPL0:
        surfaceParam.surfType        = SurfaceTypeFcTarget0;
        surfaceParam.isOutput        = true;
        surfaceParam.combineChannelY = true;
        break;
    case FC_FP_FASTEXPRESS_INPUTPL1:
    case FC_FP_FASTEXPRESS_OUTPUTPL1:
        surfaceParam.surfType = SurfaceTypeInvalid;
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::SetupSingleFcFastExpressKrnArg(L0_FC_FP_KRN_IMAGE_PARAM &imageParams, L0_FC_FP_KRN_TARGET_PARAM &targetParam, uint32_t localSize[3], uint32_t globalSize[3], KRN_ARG &krnArg, bool &bInit)
{
    switch (krnArg.uIndex)
    {
    case FC_FP_FASTEXPRESS_IMAGEPARAM:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(imageParams));
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &imageParams, sizeof(imageParams));
        break;
    case FC_FP_FASTEXPRESS_OUTPUTPARAM:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, sizeof(targetParam));
        MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, &targetParam, sizeof(targetParam));
        break;
    case FC_FP_FASTEXPRESS_INLINE_SAMPLER_LINEAR_CLAMP_EDGE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = MHW_SAMPLER_FILTER_BILINEAR;
        break;
    case FC_FP_FASTEXPRESS_INLINE_SAMPLER_NEAREST_CLAMP_EDGE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        *(uint32_t *)krnArg.pData = MHW_SAMPLER_FILTER_NEAREST;
        break;
    case FC_FP_FASTEXPRESS_GLOBAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = globalSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = globalSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = globalSize[2];
        break;
    case FC_FP_FASTEXPRESS_ENQUEUED_LOCAL_SIZE:
    case FC_FP_FASTEXPRESS_LOCAL_SIZE:
        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        static_cast<uint32_t *>(krnArg.pData)[0] = localSize[0];
        static_cast<uint32_t *>(krnArg.pData)[1] = localSize[1];
        static_cast<uint32_t *>(krnArg.pData)[2] = localSize[2];
        break;
    default:
        bInit = false;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::GenerateFastExpressInputOutputParam(L0_FC_COMP_PARAM &compParam, L0_FC_FP_KRN_IMAGE_PARAM &imageParam, L0_FC_FP_KRN_TARGET_PARAM &targetParam)
{
    VP_FUNC_CALL();
    L0_FC_LAYER_PARAM &inputLayer  = compParam.inputLayersParam[0];
    L0_FC_LAYER_PARAM &outputLayer = compParam.outputLayerParam;
    VP_PUBLIC_CHK_NULL_RETURN(inputLayer.surf);
    VP_PUBLIC_CHK_NULL_RETURN(inputLayer.surf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputLayer.surf);
    VP_PUBLIC_CHK_NULL_RETURN(outputLayer.surf->osSurface);

    uint32_t inputWidth  = MOS_MIN(static_cast<uint32_t>(inputLayer.surf->osSurface->dwWidth), static_cast<uint32_t>(inputLayer.surf->rcSrc.right));
    uint32_t inputHeight = MOS_MIN(static_cast<uint32_t>(inputLayer.surf->osSurface->dwHeight), static_cast<uint32_t>(inputLayer.surf->rcSrc.bottom));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertCscToKrnParam(inputLayer.surf->ColorSpace, compParam.mainCSpace, imageParam.csc));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertInputChannelIndicesToKrnParam(inputLayer.surf->osSurface->Format, imageParam.inputChannelIndices));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertScalingRotToKrnParam(inputLayer.surf->rcSrc, inputLayer.surf->rcDst, inputLayer.scalingMode, inputWidth, inputHeight, inputLayer.rotation, imageParam.scaleParam, imageParam.controlSetting.samplerType, imageParam.coordShift));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertChromaUpsampleToKrnParam(inputLayer.surf->osSurface->Format, inputLayer.surf->ChromaSiting, inputLayer.scalingMode, inputWidth, inputHeight, imageParam.coordShift.chromaShiftX, imageParam.coordShift.chromaShiftY, imageParam.controlSetting.isChromaShift));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertPlaneNumToKrnParam(inputLayer.surf->osSurface->Format, true, imageParam.inputPlaneNum));

    VP_PUBLIC_CHK_STATUS_RETURN(ConvertAlignedTrgRectToKrnParam(inputLayer.surf, outputLayer.surf, compParam.enableColorFill, targetParam));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertPlaneNumToKrnParam(outputLayer.surf->osSurface->Format, false, targetParam.planeNumber));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertOuputChannelIndicesToKrnParam(outputLayer.surf->osSurface->Format, targetParam.dynamicChannelIndices));
    targetParam.combineChannelIndices[0] = 0;
    targetParam.combineChannelIndices[1] = 1;
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertTargetRoiToKrnParam(outputLayer.surf->rcDst, outputLayer.surf->osSurface->dwWidth, outputLayer.surf->osSurface->dwHeight, targetParam.targetROI));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertChromaDownsampleToKrnParam(outputLayer.surf->osSurface->Format, outputLayer.surf->ChromaSiting, targetParam.chromaSitingFactor, targetParam.controlSetting.hitSecPlaneFactorX, targetParam.controlSetting.hitSecPlaneFactorY));
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertColorFillToKrnParam(compParam.enableColorFill, compParam.colorFillParams, compParam.mainCSpace, targetParam.controlSetting.isColorFill, targetParam.background));
    //ConvertAlphaToKrnParam must be called after ConvertColorFillToKrnParam
    uint8_t alphaLayerIndex = 0;
    VP_PUBLIC_CHK_STATUS_RETURN(ConvertAlphaToKrnParam(compParam.bAlphaCalculateEnable, compParam.compAlpha, targetParam.background[3], alphaLayerIndex, targetParam.alpha));
    targetParam.controlSetting.alphaLayerIndex = (alphaLayerIndex != 0);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::ConvertAlignedTrgRectToKrnParam(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, bool enableColorFill, L0_FC_FP_KRN_TARGET_PARAM &targetParam)
{
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);


    RECT alignedRect = {};
    if (enableColorFill)
    {
        alignedRect.left   = MOS_ALIGN_FLOOR(MOS_MAX(0, outputSurf->rcDst.left), 8);
        alignedRect.right  = MOS_ALIGN_CEIL(MOS_MIN(outputSurf->osSurface->dwWidth, (uint64_t)outputSurf->rcDst.right), 8);
        alignedRect.top    = MOS_ALIGN_FLOOR(MOS_MAX(0, outputSurf->rcDst.top), 4);
        alignedRect.bottom = MOS_ALIGN_CEIL(MOS_MIN(outputSurf->osSurface->dwHeight, (uint64_t)outputSurf->rcDst.bottom), 4);
    }
    else
    {
        alignedRect.left   = MOS_ALIGN_FLOOR(MOS_MAX(0, inputSurf->rcDst.left), 8);
        alignedRect.right  = MOS_ALIGN_CEIL(MOS_MIN(outputSurf->osSurface->dwWidth, (uint64_t)inputSurf->rcDst.right), 8);
        alignedRect.top    = MOS_ALIGN_FLOOR(MOS_MAX(0, inputSurf->rcDst.top), 4);
        alignedRect.bottom = MOS_ALIGN_CEIL(MOS_MIN(outputSurf->osSurface->dwHeight, (uint64_t)inputSurf->rcDst.bottom), 4);
    }
    targetParam.alignedTrgRectStart.x     = static_cast<uint16_t>(alignedRect.left);
    targetParam.alignedTrgRectStart.y     = static_cast<uint16_t>(alignedRect.top);
    targetParam.alignedTrgRectSize.width  = static_cast<uint16_t>(alignedRect.right - alignedRect.left) / 2;
    targetParam.alignedTrgRectSize.height = static_cast<uint16_t>(alignedRect.bottom - alignedRect.top) / 2;


    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::SetPerfTag(L0_FC_COMP_PARAM& compParam, VPHAL_PERFTAG& perfTag)
{
    bool rotation = false;
    bool primary  = false;

    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        L0_FC_LAYER_PARAM &layer = compParam.inputLayersParam[i];
        if (layer.surf && layer.surf->SurfType == SURF_IN_PRIMARY)
        {
            primary = true;
        }
        if (layer.rotation != VPHAL_ROTATION_IDENTITY)
        {
            rotation = true;
        }
    }
    if (rotation)
    {
        perfTag = VPHAL_PERFTAG(VPHAL_ROT + compParam.layerNumber);
    }
    else if (primary)
    {
        perfTag = VPHAL_PERFTAG(VPHAL_PRI + compParam.layerNumber);
    }
    else
    {
        perfTag = VPHAL_PERFTAG(VPHAL_NONE + compParam.layerNumber);
    }

    return MOS_STATUS_SUCCESS;
}

void VpL0FcFilter::PrintCompParam(L0_FC_COMP_PARAM& compParam)
{
    VP_FUNC_CALL();
#if (_DEBUG || _RELEASE_INTERNAL)
    VP_PUBLIC_NORMALMESSAGE("L0 FC CompParam: Layer Number %u", compParam.layerNumber);
    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        PrintCompLayerParam(i, true, compParam.inputLayersParam[i]);
    }
    PrintCompLayerParam(0, false, compParam.outputLayerParam);

    VP_PUBLIC_NORMALMESSAGE("L0 FC CompParam: mainCSpace %d", compParam.mainCSpace);
    VP_PUBLIC_NORMALMESSAGE("L0 FC CompParam: enableCalculateAlpha %d, alphaMode %d, alpha %f", compParam.bAlphaCalculateEnable, compParam.compAlpha.AlphaMode, compParam.compAlpha.fAlpha);

    VPHAL_COLOR_SAMPLE_8 color = {};
    color.dwValue              = compParam.colorFillParams.Color;
    VP_PUBLIC_NORMALMESSAGE("L0 FC CompParam: enableColorFill %d, CSpace %d", compParam.enableColorFill, compParam.colorFillParams.CSpace);
    VP_PUBLIC_NORMALMESSAGE("L0 FC CompParam: colorFill R %d, G %d, B %d, A %d", color.R, color.G, color.B, color.A);
    VP_PUBLIC_NORMALMESSAGE("L0 FC CompParam: colorFill Y %d, U %d, V %d, A %d", color.Y, color.U, color.V, color.a);
    VP_PUBLIC_NORMALMESSAGE("L0 FC CompParam: colorFill YY %d, Cr %d, Cb %d, A %d", color.YY, color.Cr, color.Cb, color.Alpha);
#endif
}

void VpL0FcFilter::PrintCompLayerParam(uint32_t index, bool isInput, L0_FC_LAYER_PARAM &layerParam)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: isInput %d, layerIndex %u, layerID %u, layerOriginID %u", isInput, index, layerParam.layerID, layerParam.layerIDOrigin);
    VP_SURFACE *surf = layerParam.surf;
    if (surf)
    {
        if (surf->osSurface)
        {
            VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: Format %d, Width %lu, Height %lu", surf->osSurface->Format, surf->osSurface->dwWidth, surf->osSurface->dwHeight);
        }
        VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: CSpace %d, ChromaSiting 0x%x", surf->ColorSpace, surf->ChromaSiting);
        VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: Src Rect, left %ld, right %ld, top %ld, bottom %ld", surf->rcSrc.left, surf->rcSrc.right, surf->rcSrc.top, surf->rcSrc.bottom);
        VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: Dst Rect, left %ld, right %ld, top %ld, bottom %ld", surf->rcDst.left, surf->rcDst.right, surf->rcDst.top, surf->rcDst.bottom);
    }

    if (isInput)
    {
        VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: ScalingMode %d, RotationMir %d", layerParam.scalingMode, layerParam.rotation);
        VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: LumaKey enable %d, low %d, high %d", layerParam.lumaKey.enabled, layerParam.lumaKey.params.LumaLow, layerParam.lumaKey.params.LumaHigh);
        VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: Blending type %d, alpha %f", layerParam.blendingParams.BlendType, layerParam.blendingParams.fAlpha);
        VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: Deinterlace enabled %d, FMD %d, SCD %d, SingleField %d, mode %d",
            layerParam.diParams.enabled,
            layerParam.diParams.params.bEnableFMD,
            layerParam.diParams.params.bSCDEnable,
            layerParam.diParams.params.bSingleField,
            layerParam.diParams.params.DIMode);
        VP_PUBLIC_NORMALMESSAGE("L0 FC CompLayerParam: Procamp enabled %d, brightness %f, contrast %f, hue %f, saturation %f",
            layerParam.procampParams.bEnabled,
            layerParam.procampParams.fBrightness,
            layerParam.procampParams.fContrast,
            layerParam.procampParams.fHue,
            layerParam.procampParams.fSaturation);
    }

    VP_PUBLIC_NORMALMESSAGE("");
#endif
}

void VpL0FcFilter::PrintKrnParam(std::vector<L0_FC_KRN_IMAGE_PARAM>& imageParams, L0_FC_KRN_TARGET_PARAM& targetParam)
{
    VP_FUNC_CALL();
#if (_DEBUG || _RELEASE_INTERNAL)
    for (uint32_t i = 0; i < imageParams.size(); ++i)
    {
        PrintKrnImageParam(i, imageParams.at(i));
    }
    PrintKrnTargetParam(targetParam);
#endif
}

void VpL0FcFilter::PrintKrnImageParam(uint32_t index, L0_FC_KRN_IMAGE_PARAM &imageParam)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam Layer Index %d", index);
    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: CSC %f %f %f %f", imageParam.csc.s0123[0], imageParam.csc.s0123[1], imageParam.csc.s0123[2], imageParam.csc.s0123[3]);
    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: CSC %f %f %f %f", imageParam.csc.s4567[0], imageParam.csc.s4567[1], imageParam.csc.s4567[2], imageParam.csc.s4567[3]);
    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: CSC %f %f %f %f", imageParam.csc.s89AB[0], imageParam.csc.s89AB[1], imageParam.csc.s89AB[2], imageParam.csc.s89AB[3]);
    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: CSC %f %f %f %f", imageParam.csc.sCDEF[0], imageParam.csc.sCDEF[1], imageParam.csc.sCDEF[2], imageParam.csc.sCDEF[3]);

    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: inputChannelIndices %u %u %u %u",
        imageParam.inputChannelIndices[0],
        imageParam.inputChannelIndices[1],
        imageParam.inputChannelIndices[2],
        imageParam.inputChannelIndices[3]);

    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: Scaling Src, startX %f, StartY %f, StrideX %f, StrideY %f", 
        imageParam.scale.src.startX,
        imageParam.scale.src.startY,
        imageParam.scale.src.strideX,
        imageParam.scale.src.strideY);
    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: Scaling Dst Rect, left %u, right %u, top %u, bottom %u",
        imageParam.scale.trg.left,
        imageParam.scale.trg.right,
        imageParam.scale.trg.top,
        imageParam.scale.trg.bottom);
    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: Rotation, indices[0] %u, indices[1] %u",
        imageParam.scale.rotateIndices[0],
        imageParam.scale.rotateIndices[1]);
    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: CoordShift isChromaShift %d, CommonShiftX %f, CommonShiftY %f,  ChromaShiftX %f, ChromaShiftY %f",
        imageParam.controlSetting.isChromaShift,
        imageParam.coordShift.commonShiftX,
        imageParam.coordShift.commonShiftY,
        imageParam.coordShift.chromaShiftX,
        imageParam.coordShift.chromaShiftY);

    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: LumaKey min %f, max %f", imageParam.lumaKey.low, imageParam.lumaKey.high);

    VP_PUBLIC_NORMALMESSAGE("L0 FC ImageParam: inputPlaneNum %u, SamplerType %d, ignoreSrcPixelAlpha %d, ignoreDstPixelAlpha %d, constAlpha %f",
        imageParam.inputPlaneNum,
        imageParam.controlSetting.samplerType,
        imageParam.controlSetting.ignoreSrcPixelAlpha,
        imageParam.controlSetting.ignoreDstPixelAlpha,
        imageParam.constAlphs);

    VP_PUBLIC_NORMALMESSAGE("");
#endif
}

void VpL0FcFilter::PrintKrnTargetParam(L0_FC_KRN_TARGET_PARAM& targetParam)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    VP_PUBLIC_NORMALMESSAGE("L0 FC TargeParam: dynamicChannelIndices %u %u %u %u",
        targetParam.dynamicChannelIndices[0],
        targetParam.dynamicChannelIndices[1],
        targetParam.dynamicChannelIndices[2],
        targetParam.dynamicChannelIndices[3]);

    VP_PUBLIC_NORMALMESSAGE("L0 FC TargeParam: Target ROI Rect, left %u, right %u, top %u, bottom %u",
        targetParam.targetROI.left,
        targetParam.targetROI.right,
        targetParam.targetROI.top,
        targetParam.targetROI.bottom);

    VP_PUBLIC_NORMALMESSAGE("L0 FC TargeParam: background %f %f %f %f",
        targetParam.background[0],
        targetParam.background[1],
        targetParam.background[2],
        targetParam.background[3]);

    VP_PUBLIC_NORMALMESSAGE("L0 FC TargeParam: chromaSiting: secPlaneFactorX %d, secPlaneFactorY %d",
        targetParam.controlSetting.hitSecPlaneFactorX,
        targetParam.controlSetting.hitSecPlaneFactorY);
    VP_PUBLIC_NORMALMESSAGE("L0 FC TargeParam: chromaSiting: chromaSitingFactorLeftTop %f, chromaSitingFactorRightTop %f, chromaSitingFactorLeftBottom %f, chromaSitingFactorRightBottom %f",
        targetParam.chromaSitingFactor[0],
        targetParam.chromaSitingFactor[1],
        targetParam.chromaSitingFactor[2],
        targetParam.chromaSitingFactor[3]);

    VP_PUBLIC_NORMALMESSAGE("L0 FC TargeParam: planeNum %u, enableColorFill %d, alphaLayerIndex %d, fAlpha %f",
        targetParam.planeNumber,
        targetParam.controlSetting.isColorFill,
        targetParam.controlSetting.alphaLayerIndex,
        targetParam.alpha);
#endif
}

void VpL0FcFilter::ReportDiffLog(const L0_FC_COMP_PARAM &compParam)
{
    VP_FUNC_CALL();
#if (_DEBUG || _RELEASE_INTERNAL)
    VpFeatureReport *vpFeatureReport = m_pvpMhwInterface->m_reporting;
    if (vpFeatureReport == nullptr)
    {
        VP_PUBLIC_ASSERTMESSAGE("vpFeatureReportExt is nullptr");
        return;
    }
    uint32_t &reportLog = vpFeatureReport->GetFeatures().diffLogL0FC;
    //check 422 input
    for (uint32_t i = 0; i < compParam.layerNumber; ++i)
    {
        const L0_FC_LAYER_PARAM &layer = compParam.inputLayersParam[i];
        VP_SURFACE              *surf  = layer.surf;
        if (surf)
        {
            if (surf->osSurface)
            {
                MOS_FORMAT format = surf->osSurface->Format;
                if (layer.scalingMode == VPHAL_SCALING_BILINEAR)
                {
                    // bilinear scaling shift difference
                    reportLog |= (1llu << 0);
                    if (format == Format_NV12 ||
                        format == Format_P010 ||
                        format == Format_P016)
                    {
                        //1 plane and 2 plane surface state read difference
                        reportLog |= (1llu << 1);
                    }
                }

                if (format == Format_400P)
                {
                    //400P has issue on legacy FC even with nearest sampler
                    reportLog |= (1llu << 2);
                }
            }   
        }
        if (layer.rotation != VPHAL_ROTATION_IDENTITY)
        {
            //rotation shift place is different with legacy FC
            reportLog |= (1llu << 3);
        }
        if (layer.diParams.enabled || layer.procampParams.bEnabled)
        {
            //di and procamp is not enabled
            reportLog |= (1llu << 4);
        }
        if (layer.lumaKey.enabled)
        {
            //luma key cases will have difference for float(L0FC) vs int(FC)
            reportLog |= (1llu << 5);
        }
    }

    VP_SURFACE *targetSurf = compParam.outputLayerParam.surf;
    if (targetSurf)
    {
        if (targetSurf->osSurface)
        {
            MOS_FORMAT format = targetSurf->osSurface->Format;
            if ((format == Format_Y210 ||
                 format == Format_Y216 ||
                 format == Format_YUY2 ||
                 format == Format_YUYV ||
                 format == Format_YVYU ||
                 format == Format_UYVY ||
                 format == Format_VYUY) &&
                targetSurf->ChromaSiting & CHROMA_SITING_HORZ_CENTER)
            {
                //422 packed no chromasiting on legacy FC
                reportLog |= (1llu << 6);
            }

            if ((format == Format_A8R8G8B8 ||
                 format == Format_A8B8G8R8 ||
                 format == Format_R10G10B10A2 ||
                 format == Format_B10G10R10A2 ||
                 format == Format_Y410) &&
                compParam.compAlpha.AlphaMode == VPHAL_ALPHA_FILL_MODE_OPAQUE)
            {
                //fixed alpha not used in legacy FC
                reportLog |= (1llu << 7);
            }
        }
    }
    if (FastExpressConditionMeet(compParam))
    {
        reportLog |= (1llu << 16);
    }
    VP_PUBLIC_NORMALMESSAGE("L0FC vs FC Difference Report Log: 0x%x", reportLog);
#endif
}

MOS_STATUS VpL0FcFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();
    if (m_executeCaps.bRender)
    {
        // create a filter Param buffer
        if (!m_renderL0FcParams)
        {
            m_renderL0FcParams = (PRENDER_L0_FC_PARAMS)MOS_New(RENDER_L0_FC_PARAMS);

            if (m_renderL0FcParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("render l0 fc Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            m_renderL0FcParams->Init();
        }

        InitKrnParams(m_renderL0FcParams->fc_kernelParams, *m_executingPipe);
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Wrong engine caps! Vebox should be used for Dn");
    }
    return MOS_STATUS_SUCCESS;
}


/****************************************************************************************************/
/*                                   HwFilter L0 Fc Parameter                                          */
/****************************************************************************************************/
HwFilterParameter *HwFilterL0FcParameter::Create(HW_FILTER_L0_FC_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterL0FcParameter *p = MOS_New(HwFilterL0FcParameter, featureType);
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

HwFilterL0FcParameter::HwFilterL0FcParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterL0FcParameter::~HwFilterL0FcParameter()
{
}

MOS_STATUS HwFilterL0FcParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterL0FcParameter::Initialize(HW_FILTER_L0_FC_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}


/****************************************************************************************************/
/*                                   Packet L0 Fc Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpRenderL0FcParameter::Create(HW_FILTER_L0_FC_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpRenderL0FcParameter *p = dynamic_cast<VpRenderL0FcParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpRenderL0FcParameter::VpRenderL0FcParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) : VpPacketParameter(packetParamFactory), m_fcFilter(pHwInterface)
{
}
VpRenderL0FcParameter::~VpRenderL0FcParameter() {}

bool VpRenderL0FcParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    VpRenderCmdPacket *renderPacket = dynamic_cast<VpRenderCmdPacket *>(pPacket);
    if (nullptr == renderPacket)
    {
        return false;
    }

    PRENDER_L0_FC_PARAMS params = m_fcFilter.GetFcParams();
    if (nullptr == params)
    {
        return false;
    }
    return MOS_SUCCEEDED(renderPacket->SetL0FcParams(params));
}

MOS_STATUS VpRenderL0FcParameter::Initialize(HW_FILTER_L0_FC_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_fcFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_fcFilter.SetExecuteEngineCaps(params.executingPipe, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_fcFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}


/****************************************************************************************************/
/*                                   Policy FC Feature Handler                                      */
/****************************************************************************************************/

MOS_STATUS PolicyL0FcFeatureHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    FeatureType type = feature.GetFeatureType();

    if (caps.bRenderHdr)
    {
        // HDR Kernel
        return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
    else
    {
        // FC
        if (FeatureTypeLumakeyOnRender == type ||
            FeatureTypeBlendingOnRender == type ||
            FeatureTypeAlphaOnRender == type ||
            FeatureTypeCscOnRender == type ||
            FeatureTypeScalingOnRender == type ||
            FeatureTypeRotMirOnRender == type ||
            FeatureTypeDiOnRender == type ||
            FeatureTypeProcampOnRender == type)
        {
            return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
        }
        else if (FeatureTypeColorFillOnRender == type)
        {
            // Only apply color fill on 1st pass.
            VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.RemoveSwFilter(&feature));
            VP_PUBLIC_CHK_STATUS_RETURN(executePipe.AddSwFilterUnordered(&feature, isInputPipe, index));
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicyL0FcFeatureHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    // feature.GetFilterEngineCaps().bEnabled should be used here instead of feature.IsFeatureEnabled(caps)
    // to ensure the feature does not be enabled.
    // feature.IsFeatureEnabled(caps) being false means the feature is not being used in current workload,
    // in which case, the feature itself may be enable and need be processed in following workloads.
    if (0 == caps.bOutputPipeFeatureInuse &&
        !feature.GetFilterEngineCaps().bEnabled &&
        (feature.GetFilterEngineCaps().forceEnableForSfc ||
            feature.GetFilterEngineCaps().forceEnableForFc))
    {
        // To avoid filter being destroyed in Policy::UpdateFeaturePipe.
        feature.GetFilterEngineCaps().usedForNextPass = 1;
    }
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy FC Handler                                              */
/****************************************************************************************************/
PolicyL0FcHandler::PolicyL0FcHandler(VP_HW_CAPS &hwCaps) : PolicyFcHandler(hwCaps)
{
    m_Type = FeatureTypeFc;
}
PolicyL0FcHandler::~PolicyL0FcHandler()
{
}

HwFilterParameter *PolicyL0FcHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        HW_FILTER_L0_FC_PARAM param   = {};
        param.type                 = m_Type;
        param.pHwInterface         = pHwInterface;
        param.vpExecuteCaps        = vpExecuteCaps;
        param.pPacketParamFactory  = &m_PacketL0ParamFactory;
        param.executingPipe        = &swFilterPipe;
        param.pfnCreatePacketParam = PolicyL0FcHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterL0FcParameter *)pHwFilterParam)->Initialize(param)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterL0FcParameter::Create(param, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}
}