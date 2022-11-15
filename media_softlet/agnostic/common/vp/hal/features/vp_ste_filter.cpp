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
//! \file     vp_ste_filter.cpp
//! \brief    Defines the common interface for Skin Tone Enhancement
//!           this file is for the base interface which is shared by all STE in driver.
//!
#include "vp_ste_filter.h"
#include "vp_vebox_cmd_packet_base.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"

namespace vp {
VpSteFilter::VpSteFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpSteFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpSteFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpSteFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_pVeboxSteParams)
    {
        MOS_FreeMemAndSetNull(m_pVeboxSteParams);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpSteFilter::SetExecuteEngineCaps(
    FeatureParamSte &steParams,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_steParams = steParams;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpSteFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();
    if (m_executeCaps.bVebox)
    {
        // create a filter Param buffer
        if (!m_pVeboxSteParams)
        {
            m_pVeboxSteParams = (PVEBOX_STE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VEBOX_STE_PARAMS));

            if (m_pVeboxSteParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("Vebox Ste Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_pVeboxSteParams, sizeof(VEBOX_STE_PARAMS));
        }

        m_pVeboxSteParams->bEnableSTE  = m_steParams.bEnableSTE;
        m_pVeboxSteParams->dwSTEFactor = m_steParams.dwSTEFactor;
        m_pVeboxSteParams->bEnableSTD  = m_steParams.bEnableSTD;
        m_pVeboxSteParams->STDParam    = m_steParams.STDParam;
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Wrong engine caps! Vebox should be used for STE");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}


/****************************************************************************************************/
/*                                   HwFilter Ste Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterSteParameter::Create(HW_FILTER_STE_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterSteParameter *p = MOS_New(HwFilterSteParameter, featureType);
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

HwFilterSteParameter::HwFilterSteParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterSteParameter::~HwFilterSteParameter()
{
}

MOS_STATUS HwFilterSteParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterSteParameter::Initialize(HW_FILTER_STE_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Vebox Ste Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpVeboxSteParameter::Create(HW_FILTER_STE_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpVeboxSteParameter *p = dynamic_cast<VpVeboxSteParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpVeboxSteParameter::VpVeboxSteParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_steFilter(pHwInterface)
{
}
VpVeboxSteParameter::~VpVeboxSteParameter() {}

bool VpVeboxSteParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    VEBOX_STE_PARAMS *params = m_steFilter.GetVeboxParams();
    if (nullptr == params)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get vebox ste params");
        return false;
    }

    VpVeboxCmdPacketBase *packet = dynamic_cast<VpVeboxCmdPacketBase *>(pPacket);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetSteParams(params));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for vebox ste");
    return false;
}

MOS_STATUS VpVeboxSteParameter::Initialize(HW_FILTER_STE_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_steFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_steFilter.SetExecuteEngineCaps(params.steParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_steFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Vebox Ste Handler                                         */
/****************************************************************************************************/
PolicyVeboxSteHandler::PolicyVeboxSteHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeSteOnVebox;
}
PolicyVeboxSteHandler::~PolicyVeboxSteHandler()
{
}

bool PolicyVeboxSteHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bSTE;
}

HwFilterParameter* PolicyVeboxSteHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe& swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{

    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterSte *swFilter = dynamic_cast<SwFilterSte *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeSteOnVebox));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamSte &param = swFilter->GetSwFilterParams();

        HW_FILTER_STE_PARAM paramSte = {};
        paramSte.type = m_Type;
        paramSte.pHwInterface = pHwInterface;
        paramSte.vpExecuteCaps = vpExecuteCaps;
        paramSte.pPacketParamFactory = &m_PacketParamFactory;
        paramSte.steParams = param;
        paramSte.pfnCreatePacketParam = PolicyVeboxSteHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterSteParameter*)pHwFilterParam)->Initialize(paramSte)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterSteParameter::Create(paramSte, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}
}
