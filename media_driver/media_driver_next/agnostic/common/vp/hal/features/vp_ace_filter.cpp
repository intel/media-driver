/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     vp_ace_filter.cpp
//! \brief    Defines the common interface for Adaptive Contrast Enhancement
//!           this file is for the base interface which is shared by all Ace in driver.
//!
#include "vp_ace_filter.h"
#include "vp_vebox_cmd_packet.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"

namespace vp {
VpAceFilter::VpAceFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpAceFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAceFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAceFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_pVeboxAceParams)
    {
        MOS_FreeMemAndSetNull(m_pVeboxAceParams);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAceFilter::SetExecuteEngineCaps(
    FeatureParamAce &aceParams,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_aceParams = aceParams;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAceFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();
    if (m_executeCaps.bVebox)
    {
        // create a filter Param buffer
        if (!m_pVeboxAceParams)
        {
            m_pVeboxAceParams = (PVEBOX_ACE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VEBOX_ACE_PARAMS));

            if (m_pVeboxAceParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("sfc Rotation Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_pVeboxAceParams, sizeof(VEBOX_ACE_PARAMS));
        }

        m_pVeboxAceParams->bEnableACE = m_aceParams.bEnableACE;
        m_pVeboxAceParams->dwAceLevel = m_aceParams.dwAceLevel;
        m_pVeboxAceParams->dwAceStrength = m_aceParams.dwAceStrength;
        m_pVeboxAceParams->bAceLevelChanged = m_aceParams.bAceLevelChanged;
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Wrong engine caps! Vebox should be used for Ace");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}


/****************************************************************************************************/
/*                                   HwFilter Ace Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterAceParameter::Create(HW_FILTER_ACE_PARAM &param, FeatureType featureType)
{
    HwFilterAceParameter *p = MOS_New(HwFilterAceParameter, featureType);
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

HwFilterAceParameter::HwFilterAceParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterAceParameter::~HwFilterAceParameter()
{
}

MOS_STATUS HwFilterAceParameter::ConfigParams(HwFilter &hwFilter)
{
    return hwFilter.ConfigAceParam(m_Params);
}

MOS_STATUS HwFilterAceParameter::Initialize(HW_FILTER_ACE_PARAM &param)
{
    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Vebox Ace Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpVeboxAceParameter::Create(HW_FILTER_ACE_PARAM &param)
{
    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpVeboxAceParameter *p = dynamic_cast<VpVeboxAceParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpVeboxAceParameter::VpVeboxAceParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_aceFilter(pHwInterface)
{
}
VpVeboxAceParameter::~VpVeboxAceParameter() {}

bool VpVeboxAceParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VpVeboxCmdPacket *pVeboxPacket = dynamic_cast<VpVeboxCmdPacket *>(pPacket);
    if (nullptr == pVeboxPacket)
    {
        return false;
    }

    VEBOX_ACE_PARAMS *pParams = m_aceFilter.GetVeboxParams();
    if (nullptr == pParams)
    {
        return false;
    }
    return MOS_SUCCEEDED(pVeboxPacket->SetAceParams(pParams));
}

MOS_STATUS VpVeboxAceParameter::Initialize(HW_FILTER_ACE_PARAM &params)
{
    VP_PUBLIC_CHK_STATUS_RETURN(m_aceFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_aceFilter.SetExecuteEngineCaps(params.aceParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_aceFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Vebox Ace Handler                                         */
/****************************************************************************************************/
PolicyVeboxAceHandler::PolicyVeboxAceHandler()
{
    m_Type = FeatureTypeAceOnVebox;
}
PolicyVeboxAceHandler::~PolicyVeboxAceHandler()
{
}

bool PolicyVeboxAceHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    return vpExecuteCaps.bAce;
}

HwFilterParameter* PolicyVeboxAceHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe& swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterAce *swFilter = dynamic_cast<SwFilterAce *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeAceOnVebox));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamAce &param = swFilter->GetSwFilterParams();

        HW_FILTER_ACE_PARAM paramAce = {};
        paramAce.type = m_Type;
        paramAce.pHwInterface = pHwInterface;
        paramAce.vpExecuteCaps = vpExecuteCaps;
        paramAce.pPacketParamFactory = &m_PacketParamFactory;
        paramAce.aceParams = param;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterAceParameter*)pHwFilterParam)->Initialize(paramAce)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterAceParameter::Create(paramAce, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}
}
