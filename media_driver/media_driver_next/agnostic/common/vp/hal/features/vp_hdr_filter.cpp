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
//! \file     vp_hdr_filter.cpp
//! \brief    Defines the common interface for Adaptive Contrast Enhancement
//!           this file is for the base interface which is shared by all Hdr in driver.
//!
#include "vp_hdr_filter.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"

namespace vp
{
VpHdrFilter::VpHdrFilter(PVP_MHWINTERFACE vpMhwInterface) : VpFilter(vpMhwInterface)
{
}

MOS_STATUS VpHdrFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_pVeboxHdrParams)
    {
        MOS_FreeMemAndSetNull(m_pVeboxHdrParams);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrFilter::CalculateEngineParams(
    FeatureParamHdr &hdrParams,
    VP_EXECUTE_CAPS  vpExecuteCaps)
{
    VP_FUNC_CALL();
    if (vpExecuteCaps.bVebox)
    {
        // create a filter Param buffer
        if (!m_pVeboxHdrParams)
        {
            m_pVeboxHdrParams = (PVEBOX_HDR_PARAMS)MOS_AllocAndZeroMemory(sizeof(VEBOX_HDR_PARAMS));

            if (m_pVeboxHdrParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("Hdr Params buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_pVeboxHdrParams, sizeof(PVEBOX_HDR_PARAMS));
        }
        m_pVeboxHdrParams->uiMaxDisplayLum      = hdrParams.uiMaxDisplayLum;
        m_pVeboxHdrParams->uiMaxContentLevelLum = hdrParams.uiMaxContentLevelLum;
        m_pVeboxHdrParams->hdrMode              = hdrParams.hdrMode;
        m_pVeboxHdrParams->srcColorSpace        = hdrParams.srcColorSpace;
        m_pVeboxHdrParams->dstColorSpace        = hdrParams.dstColorSpace;
        m_pVeboxHdrParams->dstFormat            = hdrParams.formatOutput;
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Not implement on render path for Hdr yet");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   HwFilter Hdr Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterHdrParameter::Create(HW_FILTER_HDR_PARAM &param, FeatureType featureType)
{
    HwFilterHdrParameter *p = MOS_New(HwFilterHdrParameter, featureType);
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

HwFilterHdrParameter::HwFilterHdrParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterHdrParameter::~HwFilterHdrParameter()
{
}

MOS_STATUS HwFilterHdrParameter::ConfigParams(HwFilter &hwFilter)
{
    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterHdrParameter::Initialize(HW_FILTER_HDR_PARAM &param)
{
    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Vebox Hdr Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpVeboxHdrParameter::Create(HW_FILTER_HDR_PARAM &param)
{
    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpVeboxHdrParameter *p = dynamic_cast<VpVeboxHdrParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpVeboxHdrParameter::VpVeboxHdrParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) : VpPacketParameter(packetParamFactory), m_HdrFilter(pHwInterface)
{
}
VpVeboxHdrParameter::~VpVeboxHdrParameter() {}

bool VpVeboxHdrParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VpVeboxCmdPacket *pVeboxPacket = dynamic_cast<VpVeboxCmdPacket *>(pPacket);
    if (nullptr == pVeboxPacket)
    {
        return false;
    }

    VEBOX_HDR_PARAMS *pParams = m_HdrFilter.GetVeboxParams();
    if (nullptr == pParams)
    {
        return false;
    }
    return MOS_SUCCEEDED(pVeboxPacket->SetHdrParams(pParams));
}

MOS_STATUS VpVeboxHdrParameter::Initialize(HW_FILTER_HDR_PARAM &params)
{
    VP_PUBLIC_CHK_STATUS_RETURN(m_HdrFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_HdrFilter.CalculateEngineParams(params.hdrParams, params.vpExecuteCaps));
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Vebox Hdr Handler                                         */
/****************************************************************************************************/
PolicyVeboxHdrHandler::PolicyVeboxHdrHandler()
{
    m_Type = FeatureTypeHdrOnVebox;
}
PolicyVeboxHdrHandler::~PolicyVeboxHdrHandler()
{
}

bool PolicyVeboxHdrHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    //Need to check if other path activated
    return vpExecuteCaps.bHDR3DLUT;
}

HwFilterParameter *PolicyVeboxHdrHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterHdr *swFilter = dynamic_cast<SwFilterHdr *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeHdrOnVebox));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamHdr &param = swFilter->GetSwFilterParams();

        HW_FILTER_HDR_PARAM paramHdr = {};
        paramHdr.type                = m_Type;
        paramHdr.pHwInterface        = pHwInterface;
        paramHdr.vpExecuteCaps       = vpExecuteCaps;
        paramHdr.pPacketParamFactory = &m_PacketParamFactory;
        paramHdr.hdrParams           = param;
        paramHdr.pfnCreatePacketParam = PolicyVeboxHdrHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterHdrParameter *)pHwFilterParam)->Initialize(paramHdr)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterHdrParameter::Create(paramHdr, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}
}  // namespace vp