/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     vp_tcc_filter.cpp
//! \brief    Defines the common interface for Total Color Control
//!           this file is for the base interface which is shared by all TCC in driver.
//!
#include "vp_tcc_filter.h"
#include "vp_vebox_cmd_packet_base.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"

namespace vp {
VpTccFilter::VpTccFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpTccFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpTccFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpTccFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_pVeboxTccParams)
    {
        MOS_FreeMemAndSetNull(m_pVeboxTccParams);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpTccFilter::SetExecuteEngineCaps(
    FeatureParamTcc &tccParams,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_tccParams = tccParams;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpTccFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();
    if (m_executeCaps.bVebox)
    {
        // create a filter Param buffer
        if (!m_pVeboxTccParams)
        {
            m_pVeboxTccParams = (PVEBOX_TCC_PARAMS)MOS_AllocAndZeroMemory(sizeof(VEBOX_TCC_PARAMS));

            if (m_pVeboxTccParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("Vebox Tcc Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_pVeboxTccParams, sizeof(VEBOX_TCC_PARAMS));
        }

        m_pVeboxTccParams->bEnableTCC = m_tccParams.bEnableTCC;
        m_pVeboxTccParams->Blue = m_tccParams.Blue;
        m_pVeboxTccParams->Red = m_tccParams.Red;
        m_pVeboxTccParams->Yellow = m_tccParams.Yellow;
        m_pVeboxTccParams->Cyan = m_tccParams.Cyan;
        m_pVeboxTccParams->Green = m_tccParams.Green;
        m_pVeboxTccParams->Magenta = m_tccParams.Magenta;
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Wrong engine caps! Vebox should be used for TCC");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}


/****************************************************************************************************/
/*                                   HwFilter Tcc Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterTccParameter::Create(HW_FILTER_TCC_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterTccParameter *p = MOS_New(HwFilterTccParameter, featureType);
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

HwFilterTccParameter::HwFilterTccParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterTccParameter::~HwFilterTccParameter()
{
}

MOS_STATUS HwFilterTccParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterTccParameter::Initialize(HW_FILTER_TCC_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Vebox Tcc Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpVeboxTccParameter::Create(HW_FILTER_TCC_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpVeboxTccParameter *p = dynamic_cast<VpVeboxTccParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpVeboxTccParameter::VpVeboxTccParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_tccFilter(pHwInterface)
{
}
VpVeboxTccParameter::~VpVeboxTccParameter() {}

bool VpVeboxTccParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    VEBOX_TCC_PARAMS *params = m_tccFilter.GetVeboxParams();
    if (nullptr == params)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get vebox tcc params");
        return false;
    }

    VpVeboxCmdPacketBase *packet = dynamic_cast<VpVeboxCmdPacketBase *>(pPacket);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetTccParams(params));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for vebox csc");
    return false;
}

MOS_STATUS VpVeboxTccParameter::Initialize(HW_FILTER_TCC_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_tccFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_tccFilter.SetExecuteEngineCaps(params.tccParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_tccFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Vebox Tcc Handler                                         */
/****************************************************************************************************/
PolicyVeboxTccHandler::PolicyVeboxTccHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeTccOnVebox;
}
PolicyVeboxTccHandler::~PolicyVeboxTccHandler()
{
}

bool PolicyVeboxTccHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bTCC;
}

HwFilterParameter* PolicyVeboxTccHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe& swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterTcc *swFilter = dynamic_cast<SwFilterTcc *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeTccOnVebox));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamTcc &param = swFilter->GetSwFilterParams();

        HW_FILTER_TCC_PARAM paramTcc = {};
        paramTcc.type = m_Type;
        paramTcc.pHwInterface = pHwInterface;
        paramTcc.vpExecuteCaps = vpExecuteCaps;
        paramTcc.pPacketParamFactory = &m_PacketParamFactory;
        paramTcc.tccParams = param;
        paramTcc.pfnCreatePacketParam = PolicyVeboxTccHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterTccParameter*)pHwFilterParam)->Initialize(paramTcc)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterTccParameter::Create(paramTcc, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}
}
