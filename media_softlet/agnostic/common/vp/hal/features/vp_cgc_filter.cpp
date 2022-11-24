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
//! \file     vp_cgc_filter.cpp
//! \brief    Defines the common interface for Color Gamut Compression and Expansion
//!           this file is for the base interface which is shared by all CGC in driver.
//!
#include "vp_cgc_filter.h"
#include "vp_vebox_cmd_packet.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"

namespace vp {
VpCgcFilter::VpCgcFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpCgcFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCgcFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCgcFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_pVeboxCgcParams)
    {
        MOS_FreeMemAndSetNull(m_pVeboxCgcParams);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCgcFilter::SetExecuteEngineCaps(
    FeatureParamCgc &cgcParams,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_cgcParams = cgcParams;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCgcFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();
    if (m_executeCaps.bVebox)
    {
        // create a filter Param buffer
        if (!m_pVeboxCgcParams)
        {
            m_pVeboxCgcParams = (PVEBOX_CGC_PARAMS)MOS_AllocAndZeroMemory(sizeof(VEBOX_CGC_PARAMS));

            if (m_pVeboxCgcParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("Vebox CGC Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_pVeboxCgcParams, sizeof(VEBOX_CGC_PARAMS));
        }

        m_pVeboxCgcParams->bEnableCGC        = m_cgcParams.GCompMode != GAMUT_MODE_NONE ? true : false;
        m_pVeboxCgcParams->GCompMode         = m_cgcParams.GCompMode;
        m_pVeboxCgcParams->bBt2020ToRGB      = m_cgcParams.bBt2020ToRGB;
        m_pVeboxCgcParams->inputColorSpace   = m_cgcParams.colorSpace;
        m_pVeboxCgcParams->outputColorSpace  = m_cgcParams.dstColorSpace;
        m_pVeboxCgcParams->bExtendedSrcGamut = m_cgcParams.bExtendedSrcGamut;
        m_pVeboxCgcParams->bExtendedDstGamut = m_cgcParams.bExtendedDstGamut;
        m_pVeboxCgcParams->dwAttenuation     = m_cgcParams.dwAttenuation;
        m_pVeboxCgcParams->displayRGBW_x[0]  = m_cgcParams.displayRGBW_x[0];
        m_pVeboxCgcParams->displayRGBW_x[1]  = m_cgcParams.displayRGBW_x[1];
        m_pVeboxCgcParams->displayRGBW_x[2]  = m_cgcParams.displayRGBW_x[2];
        m_pVeboxCgcParams->displayRGBW_x[3]  = m_cgcParams.displayRGBW_x[3];
        m_pVeboxCgcParams->displayRGBW_y[0]  = m_cgcParams.displayRGBW_y[0];
        m_pVeboxCgcParams->displayRGBW_y[1]  = m_cgcParams.displayRGBW_y[1];
        m_pVeboxCgcParams->displayRGBW_y[2]  = m_cgcParams.displayRGBW_y[2];
        m_pVeboxCgcParams->displayRGBW_y[3]  = m_cgcParams.displayRGBW_y[3];
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Wrong engine caps! Vebox should be used for CGC");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}


/****************************************************************************************************/
/*                                   HwFilter Tcc Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterCgcParameter::Create(HW_FILTER_CGC_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterCgcParameter *p = MOS_New(HwFilterCgcParameter, featureType);
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

HwFilterCgcParameter::HwFilterCgcParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterCgcParameter::~HwFilterCgcParameter()
{
}

MOS_STATUS HwFilterCgcParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterCgcParameter::Initialize(HW_FILTER_CGC_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Vebox Tcc Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpVeboxCgcParameter::Create(HW_FILTER_CGC_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpVeboxCgcParameter *p = dynamic_cast<VpVeboxCgcParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpVeboxCgcParameter::VpVeboxCgcParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_cgcFilter(pHwInterface)
{
}
VpVeboxCgcParameter::~VpVeboxCgcParameter() {}

bool VpVeboxCgcParameter::SetPacketParam(VpCmdPacket *_packet)
{
    VP_FUNC_CALL();

    VEBOX_CGC_PARAMS *params = m_cgcFilter.GetVeboxParams();
    if (nullptr == params)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get Vebox cgc params");
        return false;
    }

    VpVeboxCmdPacketBase *packet = dynamic_cast<VpVeboxCmdPacketBase *>(_packet);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetCgcParams(params));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for Vebox cgc");
    return false;
}

MOS_STATUS VpVeboxCgcParameter::Initialize(HW_FILTER_CGC_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_cgcFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_cgcFilter.SetExecuteEngineCaps(params.cgcParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_cgcFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Vebox Tcc Handler                                         */
/****************************************************************************************************/
PolicyVeboxCgcHandler::PolicyVeboxCgcHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeCgcOnVebox;
}
PolicyVeboxCgcHandler::~PolicyVeboxCgcHandler()
{
}

bool PolicyVeboxCgcHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bCGC;
}

HwFilterParameter* PolicyVeboxCgcHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe& swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterCgc *swFilter = dynamic_cast<SwFilterCgc *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeCgcOnVebox));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamCgc &param = swFilter->GetSwFilterParams();

        HW_FILTER_CGC_PARAM paramCgc = {};
        paramCgc.type = m_Type;
        paramCgc.pHwInterface = pHwInterface;
        paramCgc.vpExecuteCaps = vpExecuteCaps;
        paramCgc.pPacketParamFactory = &m_PacketParamFactory;
        paramCgc.cgcParams = param;
        paramCgc.pfnCreatePacketParam = PolicyVeboxCgcHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterCgcParameter*)pHwFilterParam)->Initialize(paramCgc)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterCgcParameter::Create(paramCgc, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}
}
