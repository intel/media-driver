/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_hdr_render_filter.cpp
//! \brief    Defines the common interface for Adaptive Contrast Enhancement
//!           this file is for the base interface which is shared by all Hdr in driver.
//!
#include "vp_hdr_render_filter.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_render_cmd_packet.h"

namespace vp
{
VpHdrRenderFilter::VpHdrRenderFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{
}

MOS_STATUS VpHdrRenderFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrRenderFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrRenderFilter::Destroy()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrRenderFilter::SetExecuteEngineCaps(
        SwFilterPipe    *executedPipe,
        VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executedPipe  = executedPipe;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrRenderFilter::CalculateEngineParams(
    FeatureParamHdr  &hdrParams,
    VP_EXECUTE_CAPS  vpExecuteCaps)
{
    VP_FUNC_CALL();
    uint32_t   i          = 0;
    VP_SURFACE *inputSrc  = nullptr;
    VP_SURFACE *targetSrc = nullptr;

    VP_PUBLIC_CHK_NULL_RETURN(m_executedPipe);

    // create a filter Param buffer
    MOS_ZeroMemory(&m_renderHdrParams, sizeof(m_renderHdrParams));

    m_renderHdrParams.kernelId                         = (VpKernelID)kernelHdrMandatory;
    m_renderHdrParams.uSourceCount                     = m_executedPipe->GetSurfaceCount(true);
    m_renderHdrParams.uTargetCount                     = m_executedPipe->GetSurfaceCount(false);
    m_renderHdrParams.pColorFillParams                 = hdrParams.pColorFillParams;
    m_renderHdrParams.uiMaxDisplayLum                  = hdrParams.uiMaxDisplayLum;


    SwFilterScaling *scaling      = dynamic_cast<SwFilterScaling *>(m_executedPipe->GetSwFilter(true, 0, FeatureTypeScaling));
    m_renderHdrParams.ScalingMode = scaling ? scaling->GetSwFilterParams().scalingMode : VPHAL_SCALING_BILINEAR;

    SwFilterRotMir *rotation      = dynamic_cast<SwFilterRotMir *>(m_executedPipe->GetSwFilter(true, 0, FeatureTypeRotMir));
    m_renderHdrParams.Rotation    = rotation ? rotation->GetSwFilterParams().rotation : VPHAL_ROTATION_IDENTITY;

    auto blending = dynamic_cast<SwFilterBlending *>(m_executedPipe->GetSwFilter(true, i, FeatureTypeBlending));
    if (blending)
    {
        auto &blendingParams              = blending->GetSwFilterParams();
        m_renderHdrParams.pBlendingParams = blendingParams.blendingParams;
    }

    for (i = 0; i < m_renderHdrParams.uSourceCount && i < VPHAL_MAX_HDR_INPUT_LAYER; ++i)
    {
        SwFilterHdr *hdrfilter = dynamic_cast<SwFilterHdr *>(m_executedPipe->GetSwFilter(true, i, FeatureTypeHdr));

        if (nullptr == hdrfilter)
        {
            continue;
        }
        auto &hdrParams = hdrfilter->GetSwFilterParams();

        if (!vpExecuteCaps.bRender || HDR_STAGE_3DLUT_KERNEL == hdrParams.stage)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        auto &surfGroup             = m_executedPipe->GetSurfacesSetting().surfGroup;

        SurfaceType surfId         = (SurfaceType)(SurfaceTypeHdrInputLayer0 + i);
        auto        inputSrcHandle = surfGroup.find(surfId);
        if (inputSrcHandle == surfGroup.end())
        {
            VP_PUBLIC_NORMALMESSAGE("surfId %d not found", surfId);
            inputSrc = nullptr;
        }
        else
        {
            inputSrc = inputSrcHandle->second;
        }
        
        if (inputSrc == nullptr)
        {
            m_renderHdrParams.InputSrc[i] = false;
            continue;
        }
        m_renderHdrParams.InputSrc[i]     = true;
        m_renderHdrParams.srcHDRParams[i] = hdrParams.srcHDRParams;
        m_renderHdrParams.uSourceBindingTableIndex[i] = VPHAL_HDR_BTINDEX_LAYER0 + i * VPHAL_HDR_BTINDEX_PER_LAYER0;
    }

    for (i = 0; i < m_renderHdrParams.uTargetCount; i++)
    {
        auto &surfGroup             = m_executedPipe->GetSurfacesSetting().surfGroup;

        SurfaceType surfId          = (SurfaceType)(SurfaceTypeHdrTarget0 + i);
        auto        targetSrcHandle = surfGroup.find(surfId);
        if (targetSrcHandle == surfGroup.end())
        {
            VP_PUBLIC_NORMALMESSAGE("surfId %d not found", surfId);
            targetSrc = nullptr;
        }
        else
        {
            targetSrc = targetSrcHandle->second;
        }
        
        if (targetSrc == nullptr)
        {
            continue;
        }
        //auto &hdrParams                               = ((SwFilterHdr *)m_executedPipe->GetSwFilter(false, 0, FeatureTypeHdr))->GetSwFilterParams();
        m_renderHdrParams.targetHDRParams[i]          = hdrParams.targetHDRParams;
        m_renderHdrParams.uTargetBindingTableIndex[i] = VPHAL_HDR_BTINDEX_RENDERTARGET + i * VPHAL_HDR_BTINDEX_PER_TARGET;
    }

    uint32_t blockWidth = 16;
    uint32_t blockHeight = 8;

    m_renderHdrParams.threadWidth = (LUT65_SEG_SIZE * 2 + blockWidth - 1) / blockWidth;
    m_renderHdrParams.threadHeight = (LUT65_SEG_SIZE * LUT65_MUL_SIZE + blockHeight - 1) / blockHeight;

    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   HwFilter Hdr Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterHdrRenderParameter::Create(HW_FILTER_HDR_RENDER_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterHdrRenderParameter *p = MOS_New(HwFilterHdrRenderParameter, featureType);
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

HwFilterHdrRenderParameter::HwFilterHdrRenderParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterHdrRenderParameter::~HwFilterHdrRenderParameter()
{
}

MOS_STATUS HwFilterHdrRenderParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterHdrRenderParameter::Initialize(HW_FILTER_HDR_RENDER_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Render Hdr Parameter                                       */
/****************************************************************************************************/
VpPacketParameter * VpRenderHdrParameter::Create(HW_FILTER_HDR_RENDER_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpRenderHdrParameter *p = dynamic_cast< VpRenderHdrParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

 VpRenderHdrParameter:: VpRenderHdrParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) : VpPacketParameter(packetParamFactory), m_HdrFilter(pHwInterface)
{
}
 VpRenderHdrParameter::~ VpRenderHdrParameter() {}

bool VpRenderHdrParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    RENDER_HDR_PARAMS *pParams = m_HdrFilter.GetRenderParams();
    if (nullptr == pParams)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get vebox hdr params");
        return false;
    }

    VpRenderCmdPacket *packet = dynamic_cast<VpRenderCmdPacket *>(pPacket);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetHdrParams(pParams));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for Vebox hdr");
    return false;

}

MOS_STATUS VpRenderHdrParameter::Initialize(HW_FILTER_HDR_RENDER_PARAM &params)
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
PolicyRenderHdrHandler::PolicyRenderHdrHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeHdrOnRender;
}
PolicyRenderHdrHandler::~PolicyRenderHdrHandler()
{
}

bool PolicyRenderHdrHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    //Need to check if other path activated
    return vpExecuteCaps.bRenderHdr;
}

HwFilterParameter *PolicyRenderHdrHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
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

        HW_FILTER_HDR_RENDER_PARAM paramHdr = {};
        paramHdr.type                = m_Type;
        paramHdr.pHwInterface        = pHwInterface;
        paramHdr.vpExecuteCaps       = vpExecuteCaps;
        paramHdr.pPacketParamFactory = &m_PacketParamFactory;
        paramHdr.hdrParams           = param;
        paramHdr.executedPipe         = &swFilterPipe;
        paramHdr.pfnCreatePacketParam = PolicyRenderHdrHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterHdrRenderParameter *)pHwFilterParam)->Initialize(paramHdr)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterHdrRenderParameter::Create(paramHdr, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicyRenderHdrHandler::LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps)
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
        // first step to process all layers when hdr kernel used, todo: add handle for >8 layers cases.
        layerIndexes.push_back(index);
    }

    return MOS_STATUS_SUCCESS;
}


}  // namespace vp
