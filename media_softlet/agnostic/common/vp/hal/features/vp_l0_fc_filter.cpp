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
#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_hal_ddi_utils.h"
#include "igvpfc_scale_args.h"
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

    for (auto &handle : m_scalingKrnArgs)
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
    m_executeCaps  = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::InitCompParams(L0_FC_KERNEL_PARAMS &compParams, SwFilterPipe &executingPipe)
{
    MOS_ZeroMemory(&compParams, sizeof(compParams));

    //L0 FC only supports scaling kernel only for now
    for (uint32_t layerIndex = 0; layerIndex < executingPipe.GetSurfaceCount(true); ++layerIndex)
    {
        SwFilterScaling   *scaling         = dynamic_cast<SwFilterScaling *>(executingPipe.GetSwFilter(true, 0, FeatureType::FeatureTypeScaling));
        L0_FC_KERNEL_PARAM scalingKrnParam = {};
        VP_PUBLIC_CHK_STATUS_RETURN(AddScalingKrn(scaling, SurfaceType(SurfaceTypeFcInputLayer0 + layerIndex), SurfaceTypeFcTarget0, scalingKrnParam));
        compParams.push_back(scalingKrnParam);
    }
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpL0FcFilter::AddScalingKrn(SwFilterScaling *scaling, SurfaceType inputSurf, SurfaceType outputSurf, L0_FC_KERNEL_PARAM &param)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);
    VP_PUBLIC_CHK_NULL_RETURN(scaling);
    RECT srcRect = scaling->GetSwFilterParams().input.rcSrc;
    RECT trgRect = scaling->GetSwFilterParams().input.rcDst;

    if (scaling->GetSwFilterParams().interlacedScalingType != ISCALING_NONE ||
        !IS_ALPHA_FORMAT_RGB8(scaling->GetSwFilterParams().formatInput) ||
        !IS_ALPHA_FORMAT_RGB8(scaling->GetSwFilterParams().formatOutput) ||
        srcRect.left != 0 ||
        srcRect.top  != 0 ||
        trgRect.left != 0 ||
        trgRect.top  != 0)
    {
        //L0 FC only support RGB32 right now
        //L0 FC only support no left/top cropping now
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    
    uint32_t    inputWidth   = MOS_MIN(scaling->GetSwFilterParams().input.dwWidth, static_cast<uint32_t>(srcRect.right - srcRect.left));
    uint32_t    inputHeight  = MOS_MIN(scaling->GetSwFilterParams().input.dwHeight, static_cast<uint32_t>(srcRect.bottom - srcRect.top));
    uint32_t    targetWidth  = scaling->GetSwFilterParams().output.dwWidth;
    uint32_t    targetHeight = scaling->GetSwFilterParams().output.dwHeight;
    uint32_t    bitNumber    = 8;
    float       shift[2]     = {VP_HW_LINEAR_SHIFT, VP_HW_LINEAR_SHIFT};
    float       offset[2]    = {VP_SAMPLER_BIAS, VP_SAMPLER_BIAS};
    uint32_t    localWidth   = 32;
    uint32_t    localHeight  = 32;
    uint32_t    localDepth   = 1;
    uint32_t    threadWidth  = targetWidth / localWidth + (targetWidth % localWidth != 0);
    uint32_t    threadHeight = targetHeight / localHeight + (targetHeight % localHeight != 0);
    KERNEL_ARGS krnArgs      = {};
    param                    = {};

    auto handle = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find("PA_444D_fc_scale");
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
    KERNEL_BTIS kernelBtis = handle->second.GetKernelBtis();
    KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

    for (auto const &kernelArg : kernelArgs)
    {
        uint32_t uIndex    = kernelArg.uIndex;
        auto     argHandle = m_scalingKrnArgs.find(uIndex);
        if (argHandle == m_scalingKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = m_scalingKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_scalingKrnArgs);
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

        switch (krnArg.uIndex)
        {
        case FC_SCALE_PA_444D_OUTPUT_IMAGE_HEIGHT:
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            *(uint32_t *)krnArg.pData = targetHeight;
            break;
        case FC_SCALE_PA_444D_OUTPUT_IMAGE_WIDTH:
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            *(uint32_t *)krnArg.pData = targetWidth;
            break;
        case FC_SCALE_PA_444D_INPUT_IMAGE_WIDTH:
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            *(uint32_t *)krnArg.pData = inputWidth;
            break;
        case FC_SCALE_PA_444D_INPUT_IMAGE_HEIGHT:
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            *(uint32_t *)krnArg.pData = inputHeight;
            break;
        case FC_SCALE_PA_444D_BITNUM:
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            *(uint32_t *)krnArg.pData = bitNumber;
            break;
        case FC_SCALE_PA_444D_SHIFT:
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, shift, sizeof(shift));
            break;
        case FC_SCALE_PA_444D_OFFSET:
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            MOS_SecureMemcpy(krnArg.pData, krnArg.uSize, offset, sizeof(offset));
            break;
        case FC_SCALE_PA_444D_INPUTIMAGESMPL:
            krnArg.uOffsetInPayload = kernelArg.uOffsetInPayload;
            break;
        case FC_SCALE_PA_444D_ENQUEUED_LOCAL_SIZE:
        case FC_SCALE_PA_444D_LOCAL_SIZE:
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
            static_cast<uint32_t *>(krnArg.pData)[0] = localWidth;
            static_cast<uint32_t *>(krnArg.pData)[1] = localHeight;
            static_cast<uint32_t *>(krnArg.pData)[2] = localDepth;
            break;
        default:
            bInit = false;
            break;
        }

        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    for (auto const &kernelBti : kernelBtis)
    {
        uint32_t uIndex    = kernelBti.first;
        auto     argHandle = m_scalingKrnArgs.find(uIndex);
        if (argHandle == m_scalingKrnArgs.end())
        {
            KRN_ARG krnArg = {};
            argHandle      = m_scalingKrnArgs.insert(std::make_pair(uIndex, krnArg)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_scalingKrnArgs);
        }
        KRN_ARG &krnArg = argHandle->second;
        krnArg.uIndex   = uIndex;
        krnArg.eArgKind = ARG_KIND_SURFACE;
        bool bInit      = true;
        if (krnArg.pData == nullptr)
        {
            krnArg.uSize = 8;
            krnArg.pData = MOS_AllocAndZeroMemory(krnArg.uSize);
            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
        }
        else
        {
            VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, 8);
            MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
        }

        switch (krnArg.uIndex)
        {
        case FC_SCALE_PA_444D_INPUT:
            *(uint32_t *)krnArg.pData = inputSurf;
            break;
        case FC_SCALE_PA_444D_OUTPUT:
            *(uint32_t *)krnArg.pData = outputSurf;
            break;
        default:
            bInit = false;
            break;
        }

        if (bInit)
        {
            krnArgs.push_back(krnArg);
        }
    }

    param.kernelArgs   = krnArgs;
    param.kernelName   = "PA_444D_fc_scale";
    param.kernelId     = kernelFcDScale444;
    param.threadWidth  = threadWidth;
    param.threadHeight = threadHeight;
    param.localWidth   = localWidth;
    param.localHeight  = localHeight;

    return MOS_STATUS_SUCCESS;
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

        InitCompParams(m_renderL0FcParams->fc_kernelParams, *m_executingPipe);
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
PolicyL0FcHandler::PolicyL0FcHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeFc;
}
PolicyL0FcHandler::~PolicyL0FcHandler()
{
}

bool PolicyL0FcHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bComposite;
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
        param.pPacketParamFactory  = &m_PacketParamFactory;
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

MOS_STATUS PolicyL0FcHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();
    VP_PUBLIC_ASSERTMESSAGE("Should not coming here!");
    return MOS_STATUS_SUCCESS;
}

bool IsInterlacedInputSupported(VP_SURFACE &input);

bool IsBobDiEnabled(SwFilterDeinterlace *di, VP_SURFACE &input);

static MOS_STATUS IsChromaSamplingNeeded(bool &isChromaUpSamplingNeeded, bool &isChromaDownSamplingNeeded, VPHAL_SURFACE_TYPE surfType, int layerIndex, MOS_FORMAT inputFormat, MOS_FORMAT outputFormat)
{
    VPHAL_COLORPACK srcColorPack = VpHalDDIUtils::GetSurfaceColorPack(inputFormat);
    VPHAL_COLORPACK dstColorPack = VpHalDDIUtils::GetSurfaceColorPack(outputFormat);

    if (SURF_IN_PRIMARY == surfType &&
        // when 3D sampler been used, PL2 chromasitting kernel does not support sub-layer chromasitting
        ((IS_PL2_FORMAT(inputFormat) && 0 == layerIndex) ||
            inputFormat == Format_YUY2))
    {
        isChromaUpSamplingNeeded   = ((srcColorPack == VPHAL_COLORPACK_420 &&
                                        (dstColorPack == VPHAL_COLORPACK_422 || dstColorPack == VPHAL_COLORPACK_444)) ||
                                    (srcColorPack == VPHAL_COLORPACK_422 && dstColorPack == VPHAL_COLORPACK_444));
        isChromaDownSamplingNeeded = ((srcColorPack == VPHAL_COLORPACK_444 &&
                                          (dstColorPack == VPHAL_COLORPACK_422 || dstColorPack == VPHAL_COLORPACK_420)) ||
                                      (srcColorPack == VPHAL_COLORPACK_422 && dstColorPack == VPHAL_COLORPACK_420));
    }
    else
    {
        isChromaUpSamplingNeeded   = false;
        isChromaDownSamplingNeeded = false;
    }

    return MOS_STATUS_SUCCESS;
}

static MOS_STATUS Get3DSamplerScalingMode(VPHAL_SCALING_MODE &scalingMode, SwFilterSubPipe &pipe, int layerIndex, VP_SURFACE &input, VP_SURFACE &output)
{
    bool isChromaUpSamplingNeeded   = false;
    bool isChromaDownSamplingNeeded = false;
    VP_PUBLIC_CHK_STATUS_RETURN(IsChromaSamplingNeeded(isChromaUpSamplingNeeded, isChromaDownSamplingNeeded, input.SurfType, layerIndex, input.osSurface->Format, output.osSurface->Format));

    SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(pipe.GetSwFilter(FeatureType::FeatureTypeScaling));

    bool iscalingEnabled = scaling ? ISCALING_INTERLEAVED_TO_INTERLEAVED == scaling->GetSwFilterParams().interlacedScalingType : false;
    bool fieldWeaving    = scaling ? ISCALING_FIELD_TO_INTERLEAVED == scaling->GetSwFilterParams().interlacedScalingType : false;

    // The rectangle in VP_SURFACE contains the rotation information.
    // The rectangle in ScalingFilter has been adjusted based on the rotation,
    // which can be used directly here.
    bool isScalingNeeded = false;
    if (scaling)
    {
        auto &scalingParamsInput = scaling->GetSwFilterParams().input;
        isScalingNeeded          = (scalingParamsInput.rcDst.right - scalingParamsInput.rcDst.left) != (scalingParamsInput.rcSrc.right - scalingParamsInput.rcSrc.left) ||
                          (scalingParamsInput.rcDst.bottom - scalingParamsInput.rcDst.top) != (scalingParamsInput.rcSrc.bottom - scalingParamsInput.rcSrc.top);
    }
    else
    {
        isScalingNeeded = false;
    }

    if (!isScalingNeeded &&
        !isChromaUpSamplingNeeded &&
        !isChromaDownSamplingNeeded &&
        (input.SampleType == SAMPLE_PROGRESSIVE || iscalingEnabled || fieldWeaving))
    {
        scalingMode = VPHAL_SCALING_NEAREST;
    }
    else
    {
        scalingMode = VPHAL_SCALING_BILINEAR;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicyL0FcHandler::AddInputLayerForProcess(bool &bSkip, std::vector<int> &layerIndexes, VPHAL_SCALING_MODE &scalingMode, int index, VP_SURFACE &input, SwFilterSubPipe &pipe, VP_SURFACE &output, VP_EXECUTE_CAPS &caps)
{
    bSkip = false;
    --m_resCounter.layers;

    if (VPHAL_PALETTE_NONE != input.Palette.PaletteType)
    {
        --m_resCounter.palettes;
    }

    SwFilterProcamp *procamp = dynamic_cast<SwFilterProcamp *>(pipe.GetSwFilter(FeatureType::FeatureTypeProcamp));
    if (procamp && procamp->IsFeatureEnabled(caps) &&
        procamp->GetSwFilterParams().procampParams &&
        procamp->GetSwFilterParams().procampParams->bEnabled)
    {
        --m_resCounter.procamp;
    }

    SwFilterLumakey *lumakey = dynamic_cast<SwFilterLumakey *>(pipe.GetSwFilter(FeatureType::FeatureTypeLumakey));
    if (lumakey)
    {
        --m_resCounter.lumaKeys;
        if (m_resCounter.lumaKeys < 0 || layerIndexes.size() > 1)
        {
            bSkip = true;
            VP_PUBLIC_NORMALMESSAGE("Scaling Info: layer %d is not selected. lumaKeys %d, layerIndexes.size() %d",
                index,
                m_resCounter.lumaKeys,
                layerIndexes.size());
            return MOS_STATUS_SUCCESS;
        }
        if (layerIndexes.size() == 1)
        {
            m_resCounter.sampler = VP_COMP_MAX_SAMPLER;
        }
    }

    SwFilterScaling     *scaling               = dynamic_cast<SwFilterScaling *>(pipe.GetSwFilter(FeatureType::FeatureTypeScaling));
    SwFilterDeinterlace *di                    = dynamic_cast<SwFilterDeinterlace *>(pipe.GetSwFilter(FeatureType::FeatureTypeDi));
    VPHAL_SAMPLE_TYPE    sampleType            = input.SampleType;
    bool                 samplerLumakeyEnabled = m_hwCaps.m_rules.isAvsSamplerSupported;

    if (nullptr == scaling)
    {
        VP_PUBLIC_ASSERTMESSAGE("Scaling Info: Scaling filter does not exist on layer %d!", index);
        VP_PUBLIC_CHK_NULL_RETURN(scaling);
    }

    scalingMode = scaling->GetSwFilterParams().scalingMode;

    // Disable AVS scaling mode
    if (!m_hwCaps.m_rules.isAvsSamplerSupported)
    {
        if (VPHAL_SCALING_AVS == scalingMode)
        {
            scalingMode = VPHAL_SCALING_BILINEAR;
        }
    }

    if (!IsInterlacedInputSupported(input))
    {
        sampleType = SAMPLE_PROGRESSIVE;
        // Disable DI
        if (di && di->IsFeatureEnabled(caps))
        {
            di->GetFilterEngineCaps().bEnabled = false;
        }
        // Disable Iscaling
        if (scaling->IsFeatureEnabled(caps) &&
            ISCALING_NONE != scaling->GetSwFilterParams().interlacedScalingType)
        {
            scaling->GetSwFilterParams().interlacedScalingType = ISCALING_NONE;
        }
    }

    // Number of AVS, but lumaKey and BOB DI needs 3D sampler instead of AVS sampler.
    if (VPHAL_SCALING_AVS == scalingMode &&
        nullptr == lumakey && !IsBobDiEnabled(di, input))
    {
        --m_resCounter.avs;
    }
    // Number of Sampler filter mode, we had better only support Nearest or Bilinear filter in one phase
    // If two filters are used together, the later filter overwrite the first and cause output quality issue.
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(Get3DSamplerScalingMode(scalingMode, pipe, layerIndexes.size(), input, output));

        // If bilinear needed for one layer, it will also be used by other layers.
        // nearest only be used if it is used by all layers.
        int32_t samplerMask = (VP_COMP_SAMPLER_BILINEAR | VP_COMP_SAMPLER_NEAREST);

        // Use sampler luma key feature only if this is not the bottom most layer
        if (samplerLumakeyEnabled && lumakey && layerIndexes.size() > 0 && !IS_PL3_FORMAT(input.osSurface->Format))
        {
            m_resCounter.sampler &= VP_COMP_SAMPLER_LUMAKEY;
        }
        else if (m_resCounter.sampler & samplerMask)
        {
            m_resCounter.sampler &= samplerMask;
        }
        else
        {
            // switch to AVS if AVS sampler is not used, decrease the count of comp phase
            // For isAvsSamplerSupported == false case, curent layer will be rejected, since m_resCounter.avs == 0.
            scalingMode = VPHAL_SCALING_AVS;
            --m_resCounter.avs;
        }
    }

    // Fails if any of the limits are reached
    // Output structure has reason why failed :-)
    // multi-passes if rotation is not the same as Layer 0 rotation
    // single pass if Primary layer needs rotation and remaining layer does not need rotation
    if (m_resCounter.layers < 0 ||
        m_resCounter.palettes < 0 ||
        m_resCounter.procamp < 0 ||
        m_resCounter.lumaKeys < 0 ||
        m_resCounter.avs < 0 ||
        m_resCounter.sampler == 0)
    {
        //Multipass
        bSkip = true;
        VP_PUBLIC_NORMALMESSAGE("Scaling Info: layer %d is not selected. layers %d, palettes %d, procamp %d, lumaKeys %d, avs %d, sampler %d",
            index,
            m_resCounter.layers,
            m_resCounter.palettes,
            m_resCounter.procamp,
            m_resCounter.lumaKeys,
            m_resCounter.avs,
            m_resCounter.sampler);
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_NORMALMESSAGE("Scaling Info: scalingMode %d is selected for layer %d", scalingMode, index);
    MT_LOG2(MT_VP_HAL_FC_SCALINGINFO, MT_NORMAL, MT_VP_HAL_FC_LAYER, index, MT_VP_HAL_SCALING_MODE, scalingMode);

    // Append source to compositing operation
    scaling->GetSwFilterParams().scalingMode = scalingMode;

    if (di)
    {
        di->GetSwFilterParams().sampleTypeInput = sampleType;
    }

    input.SampleType = sampleType;
    layerIndexes.push_back(index);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicyL0FcHandler::RemoveTransparentLayers(SwFilterPipe &featurePipe)
{
    for (uint32_t i = 0; i < featurePipe.GetSurfaceCount(true); ++i)
    {
        SwFilterSubPipe *subpipe = featurePipe.GetSwFilterSubPipe(true, i);

        auto blending = dynamic_cast<SwFilterBlending *>(featurePipe.GetSwFilter(true, i, FeatureTypeBlending));
        if (nullptr == blending)
        {
            continue;
        }

        auto &param = blending->GetSwFilterParams();

        //-----------------------------------
        // Alpha blending optimization.
        // If Constant blending and one of the following is true, disable blending.
        // If Src+Constant blending and one of the following is true, fall back to Src blending.
        // Condition; alpha <= 0. Layer is 100% transparent.
        // Condition; alpha >= 1. Layer is 100% opaque.
        //-----------------------------------
        if (param.blendingParams &&
            ((param.blendingParams->BlendType == BLEND_CONSTANT) ||
                (param.blendingParams->BlendType == BLEND_CONSTANT_SOURCE) ||
                (param.blendingParams->BlendType == BLEND_CONSTANT_PARTIAL)))
        {
            float fAlpha = param.blendingParams->fAlpha;

            // Don't render layer with alpha <= 0.0f
            if (fAlpha <= 0.0f)
            {
                VP_PUBLIC_NORMALMESSAGE("Layer %d skipped: BlendType %d, fAlpha %d",
                    i,
                    param.blendingParams->BlendType,
                    param.blendingParams->fAlpha);
                VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.DestroySurface(true, i));
            }
        }
    }
    featurePipe.Update();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PolicyL0FcHandler::LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps)
{
    layerIndexes.clear();
    m_resCounter.Reset(m_hwCaps.m_rules.isAvsSamplerSupported);

    VP_PUBLIC_CHK_STATUS_RETURN(RemoveTransparentLayers(featurePipe));

    bool        skip                      = false;
    VP_SURFACE *output                    = featurePipe.GetSurface(false, 0);
    bool        bilinearInUseFor3DSampler = false;
    VP_PUBLIC_CHK_NULL_RETURN(output);

    for (uint32_t i = 0; i < featurePipe.GetSurfaceCount(true); ++i)
    {
        VPHAL_SCALING_MODE scalingMode = VPHAL_SCALING_NEAREST;
        VP_SURFACE        *input       = featurePipe.GetSurface(true, i);
        SwFilterSubPipe   *subpipe     = featurePipe.GetSwFilterSubPipe(true, i);
        VP_PUBLIC_CHK_NULL_RETURN(input);
        VP_PUBLIC_CHK_NULL_RETURN(subpipe);
        VP_PUBLIC_CHK_STATUS_RETURN(AddInputLayerForProcess(skip, layerIndexes, scalingMode, i, *input, *subpipe, *output, caps));
        if (skip)
        {
            break;
        }

        if (VPHAL_SCALING_BILINEAR == scalingMode)
        {
            bilinearInUseFor3DSampler = true;
        }
    }

    // Use bilinear for layers, which is using nearest.
    if (bilinearInUseFor3DSampler)
    {
        for (uint32_t i = 0; i < layerIndexes.size(); ++i)
        {
            SwFilterSubPipe *subpipe = featurePipe.GetSwFilterSubPipe(true, layerIndexes[i]);
            VP_PUBLIC_CHK_NULL_RETURN(subpipe);
            SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(subpipe->GetSwFilter(FeatureType::FeatureTypeScaling));
            if (scaling && VPHAL_SCALING_NEAREST == scaling->GetSwFilterParams().scalingMode)
            {
                scaling->GetSwFilterParams().scalingMode = VPHAL_SCALING_BILINEAR;
                VP_PUBLIC_NORMALMESSAGE("Scaling Info: Force nearest to bilinear for layer %d (%d)", layerIndexes[i], i);
                MT_LOG3(MT_VP_HAL_FC_SCALINGINFO, MT_NORMAL, MT_VP_HAL_FC_LAYER, layerIndexes[i], MT_VP_HAL_SCALING_MODE, VPHAL_SCALING_NEAREST, MT_VP_HAL_SCALING_MODE_FORCE, VPHAL_SCALING_BILINEAR);
            }
        }
    }

    // No procamp in target being used.
    return MOS_STATUS_SUCCESS;
}
}