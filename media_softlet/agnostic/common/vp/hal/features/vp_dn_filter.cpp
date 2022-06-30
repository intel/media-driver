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
//! \file     vp_dn_filter.cpp
//! \brief    Defines the common interface for denoise
//!           this file is for the base interface which is shared by all denoise in driver.
//!
#include "vp_csc_filter.h"
#include "vp_vebox_cmd_packet.h"
#include "vp_render_cmd_packet.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"

namespace vp {
VpDnFilter::VpDnFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpDnFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpDnFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpDnFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_veboxDnParams)
    {
        MOS_FreeMemory(m_veboxDnParams);
        m_veboxDnParams = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpDnFilter::SetExecuteEngineCaps(
    FeatureParamDenoise &dnParams,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_dnParams = dnParams;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpDnFilter::CalculateRenderDnHVSParams()
{
    VP_FUNC_CALL();
    VP_SURFACE *pStatisticsOutput;
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpDnFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();
    if (m_executeCaps.bVebox)
    {
        // create a filter Param buffer
        if (!m_veboxDnParams)
        {
            m_veboxDnParams = (PVEBOX_DN_PARAMS)MOS_AllocAndZeroMemory(sizeof(VEBOX_DN_PARAMS));

            if (m_veboxDnParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("sfc Rotation Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_veboxDnParams, sizeof(VEBOX_DN_PARAMS));
        }

        m_veboxDnParams->bDnEnabled         = m_executeCaps.bDN;
        m_veboxDnParams->bChromaDenoise     = m_dnParams.denoiseParams.bEnableChroma;
        m_veboxDnParams->bAutoDetect        = m_dnParams.denoiseParams.bAutoDetect;
        m_veboxDnParams->fDenoiseFactor     = m_dnParams.denoiseParams.fDenoiseFactor;
        m_veboxDnParams->NoiseLevel         = m_dnParams.denoiseParams.NoiseLevel;
        m_veboxDnParams->bEnableHVSDenoise  = m_dnParams.denoiseParams.bEnableHVSDenoise;
        m_veboxDnParams->HVSDenoise         = m_dnParams.denoiseParams.HVSDenoise;
        m_veboxDnParams->bProgressive       = SAMPLE_PROGRESSIVE == m_dnParams.sampleTypeInput;
    }
    else if (m_executeCaps.bRender && DN_STAGE_HVS_KERNEL == m_dnParams.stage)
    {
        // create a filter Param buffer
        MOS_ZeroMemory(&m_renderDnHVSParams, sizeof(RENDER_DN_HVS_CAL_PARAMS));
        m_renderDnHVSParams.kernelId              = (VpKernelID)kernelHVSCalc;

        m_renderDnHVSParams.threadWidth  = 1;
        m_renderDnHVSParams.threadHeight = 1;

        KRN_ARG krnArg  = {};
        krnArg.uIndex   = 0;
        krnArg.eArgKind = ARG_KIND_SURFACE;
        krnArg.uSize    = 4;
        krnArg.pData    = &m_surfTypeHVSTable;
        m_renderDnHVSParams.kernelArgs.push_back(krnArg);

        // Other Render's params will add in SetKernelArgs()
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Wrong engine caps! Vebox should be used for Dn");
    }
    return MOS_STATUS_SUCCESS;
}


/****************************************************************************************************/
/*                                   HwFilter Dn Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterDnParameter::Create(HW_FILTER_DN_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterDnParameter *p = MOS_New(HwFilterDnParameter, featureType);
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

HwFilterDnParameter::HwFilterDnParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterDnParameter::~HwFilterDnParameter()
{
}

MOS_STATUS HwFilterDnParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterDnParameter::Initialize(HW_FILTER_DN_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Vebox Dn Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpVeboxDnParameter::Create(HW_FILTER_DN_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpVeboxDnParameter *p = dynamic_cast<VpVeboxDnParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpVeboxDnParameter::VpVeboxDnParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_dnFilter(pHwInterface)
{
}
VpVeboxDnParameter::~VpVeboxDnParameter() {}

bool VpVeboxDnParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    //VpVeboxCmdPacket *pVeboxPacket = dynamic_cast<VpVeboxCmdPacket *>(pPacket);
    //if (nullptr == pVeboxPacket)
    //{
    //    return false;
    //}

    //VEBOX_DN_PARAMS *pParams = m_dnFilter.GetVeboxParams();
    //if (nullptr == pParams)
    //{
    //    return false;
    //}
    //return MOS_SUCCEEDED(pVeboxPacket->SetDnParams(pParams));

    VEBOX_DN_PARAMS *pParams = m_dnFilter.GetVeboxParams();
    if (nullptr == pParams)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get Vebox DN params");
        return false;
    }

    VpVeboxCmdPacketBase *packet = dynamic_cast<VpVeboxCmdPacketBase *>(pPacket);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetDnParams(pParams));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for Vebox DN");
    return false;
}

MOS_STATUS VpVeboxDnParameter::Initialize(HW_FILTER_DN_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_dnFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_dnFilter.SetExecuteEngineCaps(params.dnParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_dnFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Vebox Dn Handler                                         */
/****************************************************************************************************/
PolicyVeboxDnHandler::PolicyVeboxDnHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeDnOnVebox;
}
PolicyVeboxDnHandler::~PolicyVeboxDnHandler()
{
}

bool PolicyVeboxDnHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bDN;
}

HwFilterParameter* PolicyVeboxDnHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe& swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterDenoise *swFilter = dynamic_cast<SwFilterDenoise *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeDnOnVebox));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamDenoise &param = swFilter->GetSwFilterParams();

        HW_FILTER_DN_PARAM paramDn = {};
        paramDn.type = m_Type;
        paramDn.pHwInterface = pHwInterface;
        paramDn.vpExecuteCaps = vpExecuteCaps;
        paramDn.pPacketParamFactory = &m_PacketParamFactory;
        paramDn.dnParams = param;
        paramDn.pfnCreatePacketParam = PolicyVeboxDnHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterDnParameter*)pHwFilterParam)->Initialize(paramDn)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterDnParameter::Create(paramDn, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}


/****************************************************************************************************/
/*                      Packet Render Dn HVS Caculation Parameter                                */
/****************************************************************************************************/
VpPacketParameter *VpRenderDnHVSCalParameter::Create(HW_FILTER_DN_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpRenderDnHVSCalParameter *p = dynamic_cast<VpRenderDnHVSCalParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpRenderDnHVSCalParameter::VpRenderDnHVSCalParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) : VpPacketParameter(packetParamFactory), m_DnFilter(pHwInterface)
{
}

VpRenderDnHVSCalParameter::~VpRenderDnHVSCalParameter()
{
}

bool VpRenderDnHVSCalParameter::SetPacketParam(VpCmdPacket *packet)
{
    VP_FUNC_CALL();

    VpRenderCmdPacket *pRenderPacket = dynamic_cast<VpRenderCmdPacket *>(packet);
    if (nullptr == pRenderPacket)
    {
        return false;
    }
    RENDER_DN_HVS_CAL_PARAMS *   params = m_DnFilter.GetRenderDnHVSParams();
    if (nullptr == params)
    {
        return false;
    }
    return MOS_SUCCEEDED(pRenderPacket->SetDnHVSParams(params));
}

MOS_STATUS VpRenderDnHVSCalParameter::Initialize(HW_FILTER_DN_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_DnFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_DnFilter.SetExecuteEngineCaps(params.dnParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_DnFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                            Policy Render DN HVS Caculation Handler                            */
/****************************************************************************************************/
PolicyRenderDnHVSCalHandler::PolicyRenderDnHVSCalHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeDnHVSCalOnRender;
}
PolicyRenderDnHVSCalHandler::~PolicyRenderDnHVSCalHandler()
{
}

bool PolicyRenderDnHVSCalHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    //Need to check if other path activated
    return vpExecuteCaps.bHVSCalc;
}

HwFilterParameter *PolicyRenderDnHVSCalHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterDenoise *swFilter = dynamic_cast<SwFilterDenoise *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeDnHVSCalOnRender));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamDenoise &param    = swFilter->GetSwFilterParams();

        HW_FILTER_DN_PARAM paramDn    = {};

        paramDn.type                 = m_Type;
        paramDn.pHwInterface         = pHwInterface;
        paramDn.vpExecuteCaps        = vpExecuteCaps;
        paramDn.pPacketParamFactory  = &m_PacketParamFactory;
        paramDn.dnParams             = param;
        paramDn.pfnCreatePacketParam = PolicyRenderDnHVSCalHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterDnParameter *)pHwFilterParam)->Initialize(paramDn)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterDnParameter::Create(paramDn, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicyRenderDnHVSCalHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    SwFilterDenoise *featureDn       = dynamic_cast<SwFilterDenoise *>(&feature);
    uint32_t         widthAlignUint  = 0;
    uint32_t         heightAlignUnit = 0;
    VP_PUBLIC_CHK_NULL_RETURN(featureDn);

    // only stage in HVS_KERNEL need update engine caps
    if (caps.bHVSCalc && featureDn->GetSwFilterParams().stage == DN_STAGE_HVS_KERNEL)
    {
        SwFilterDenoise *filter2ndPass = featureDn;
        SwFilterDenoise *filter1ndPass = (SwFilterDenoise *)feature.Clone();

        VP_PUBLIC_CHK_NULL_RETURN(filter1ndPass);
        VP_PUBLIC_CHK_NULL_RETURN(filter2ndPass);

        filter1ndPass->GetFilterEngineCaps() = filter2ndPass->GetFilterEngineCaps();
        filter1ndPass->SetFeatureType(filter2ndPass->GetFeatureType());

        FeatureParamDenoise &params2ndPass = filter2ndPass->GetSwFilterParams();
        FeatureParamDenoise &params1stPass = filter1ndPass->GetSwFilterParams();

        params2ndPass.stage = DN_STAGE_VEBOX_HVS_UPDATE;

        // Clear engine caps for filter in 2nd pass.
        filter2ndPass->SetFeatureType(FeatureTypeDn);
        filter2ndPass->SetRenderTargetType(RenderTargetTypeSurface);
        filter2ndPass->GetFilterEngineCaps().RenderNeeded = 0;
        filter2ndPass->GetFilterEngineCaps().isolated     = 0;

        widthAlignUint  = featureDn->GetSwFilterParams().widthAlignUnitInput;
        heightAlignUnit = featureDn->GetSwFilterParams().heightAlignUnitInput;

        widthAlignUint           = MOS_ALIGN_CEIL(widthAlignUint, 2);

        if (featureDn->GetSwFilterParams().formatInput == Format_NV12 ||
            featureDn->GetSwFilterParams().formatInput == Format_P010 ||
            featureDn->GetSwFilterParams().formatInput == Format_P016)
        {
            heightAlignUnit = MOS_ALIGN_CEIL(heightAlignUnit, 4);
        }
        else
        {
            heightAlignUnit = MOS_ALIGN_CEIL(heightAlignUnit, 2);
        }

        if (MOS_IS_ALIGNED(featureDn->GetSwFilterParams().heightInput, heightAlignUnit))
        {
            filter2ndPass->GetFilterEngineCaps().bEnabled = 1;
            filter2ndPass->GetFilterEngineCaps().VeboxNeeded = 1;
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("Denoise Feature is disabled since heightInput (%d) not being %d aligned.", featureDn->GetSwFilterParams().heightInput, heightAlignUnit);
        }

        featureDn->GetSwFilterParams().widthAlignUnitInput = widthAlignUint;
        featureDn->GetSwFilterParams().heightAlignUnitInput = heightAlignUnit;
        executePipe.AddSwFilterUnordered(filter1ndPass, isInputPipe, index);
    }
    else
    {
        return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }

    return MOS_STATUS_SUCCESS;
}
}
