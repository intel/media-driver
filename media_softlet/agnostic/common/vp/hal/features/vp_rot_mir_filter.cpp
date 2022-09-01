/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     vp_rot_mir_filter.cpp
//! \brief    Defines the common interface for rotation/mirror
//!           this file is for the base interface which is shared by rotation/mirror in driver.
//!

#include "vp_rot_mir_filter.h"
#include "vp_vebox_cmd_packet_base.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"

using namespace vp;

VpRotMirFilter::VpRotMirFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpRotMirFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRotMirFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRotMirFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_sfcRotMirParams)
    {
        MOS_FreeMemory(m_sfcRotMirParams);
        m_sfcRotMirParams = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRotMirFilter::SetExecuteEngineCaps(
    FeatureParamRotMir      &rotMirParams,
    VP_EXECUTE_CAPS         vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executeCaps   = vpExecuteCaps;
    m_rotMirParams = rotMirParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRotMirFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();

    if (m_executeCaps.bSFC)
    {
        // create a filter Param buffer
        if (!m_sfcRotMirParams)
        {
            m_sfcRotMirParams = (PSFC_ROT_MIR_PARAMS)MOS_AllocAndZeroMemory(sizeof(SFC_ROT_MIR_PARAMS));

            if (m_sfcRotMirParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("sfc Rotation Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_sfcRotMirParams, sizeof(SFC_ROT_MIR_PARAMS));
        }

        VP_PUBLIC_CHK_STATUS_RETURN(SetRotationAndMirrowParams(m_rotMirParams.rotation));
    }
    else
    {

    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRotMirFilter::SetRotationAndMirrowParams(
    VPHAL_ROTATION Rotation)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRotMirParams);

    if (Rotation <= VPHAL_ROTATION_270)
    {
        // Rotation only
        m_sfcRotMirParams->rotationMode  = GetRotationParam(Rotation);
        m_sfcRotMirParams->bMirrorEnable = false;
    }
    else if (Rotation <= VPHAL_MIRROR_VERTICAL)
    {
        // Mirror only
        m_sfcRotMirParams->mirrorType    = GetRotationParam(Rotation) - 4;
        m_sfcRotMirParams->rotationMode  = VPHAL_ROTATION_IDENTITY;
        m_sfcRotMirParams->bMirrorEnable = true;
    }
    else
    {
        // Rotation + Mirror
        m_sfcRotMirParams->mirrorType    = MHW_MIRROR_HORIZONTAL;
        m_sfcRotMirParams->rotationMode  = GetRotationParam(Rotation);
        m_sfcRotMirParams->bMirrorEnable = true;
    }

    return MOS_STATUS_SUCCESS;
}

VPHAL_ROTATION VpRotMirFilter::GetRotationParam(VPHAL_ROTATION Rotation)
{
    VP_FUNC_CALL();

    switch (Rotation)
    {
    case VPHAL_ROTATION_90:
        return VPHAL_ROTATION_90;                         // 90 Degree Rotation

    case VPHAL_ROTATION_180:
        return VPHAL_ROTATION_180;                        // 180 Degree Rotation

    case VPHAL_ROTATION_270:
        return VPHAL_ROTATION_270;                        // 270 Degree Rotation

    case VPHAL_MIRROR_HORIZONTAL:
        return VPHAL_MIRROR_HORIZONTAL;                   // Horizontal Mirror

    case VPHAL_MIRROR_VERTICAL:
        return VPHAL_MIRROR_VERTICAL;                     // Vertical Mirror

    case VPHAL_ROTATE_90_MIRROR_VERTICAL:
        return VPHAL_ROTATION_270;                        // 270 Degree rotation and Horizontal Mirror

    case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
        return VPHAL_ROTATION_90;                         // 90 Degree rotation and Horizontal Mirror

    default:
        return VPHAL_ROTATION_IDENTITY;
    }
}

/****************************************************************************************************/
/*                          HwFilter Rotation and Mirror Parameter                                  */
/****************************************************************************************************/
HwFilterParameter *HwFilterRotMirParameter::Create(HW_FILTER_ROT_MIR_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterRotMirParameter *p = MOS_New(HwFilterRotMirParameter, featureType);
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

HwFilterRotMirParameter::HwFilterRotMirParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterRotMirParameter::~HwFilterRotMirParameter()
{
}

MOS_STATUS HwFilterRotMirParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterRotMirParameter::Initialize(HW_FILTER_ROT_MIR_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                          Packet Sfc Rotation and Mirror Parameter                                */
/****************************************************************************************************/
VpPacketParameter *VpSfcRotMirParameter::Create(HW_FILTER_ROT_MIR_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpSfcRotMirParameter *p = dynamic_cast<VpSfcRotMirParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpSfcRotMirParameter::VpSfcRotMirParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_RotMirFilter(pHwInterface)
{
}
VpSfcRotMirParameter::~VpSfcRotMirParameter() {}

bool VpSfcRotMirParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    SFC_ROT_MIR_PARAMS *pParams = m_RotMirFilter.GetSfcParams();
    if (nullptr == pParams)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get sfc rotation/mirror params");
        return false;
    }

    VpVeboxCmdPacketBase *packet = dynamic_cast<VpVeboxCmdPacketBase *>(pPacket);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetSfcRotMirParams(pParams));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for sfc rotation/mirror");
    return false;
}

MOS_STATUS VpSfcRotMirParameter::Initialize(HW_FILTER_ROT_MIR_PARAM & params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_RotMirFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_RotMirFilter.SetExecuteEngineCaps(params.rotMirParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_RotMirFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                        Policy Sfc Rotation and Mirror Handler                                    */
/****************************************************************************************************/
PolicySfcRotMirHandler::PolicySfcRotMirHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeRotMirOnSfc;
}
PolicySfcRotMirHandler::~PolicySfcRotMirHandler()
{
}

bool PolicySfcRotMirHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    return vpExecuteCaps.bSfcRotMir;
}

HwFilterParameter *PolicySfcRotMirHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterRotMir *swFilter = dynamic_cast<SwFilterRotMir *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeRotMirOnSfc));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamRotMir &param = swFilter->GetSwFilterParams();

        HW_FILTER_ROT_MIR_PARAM paramRotMir = {};
        paramRotMir.type = m_Type;
        paramRotMir.pHwInterface = pHwInterface;
        paramRotMir.vpExecuteCaps = vpExecuteCaps;
        paramRotMir.pPacketParamFactory = &m_PacketParamFactory;
        paramRotMir.rotMirParams = param;
        paramRotMir.pfnCreatePacketParam = PolicySfcRotMirHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterRotMirParameter*)pHwFilterParam)->Initialize(paramRotMir)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterRotMirParameter::Create(paramRotMir, m_Type);
        }

        return pHwFilterParam;
   }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicySfcRotMirHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    SwFilterRotMir *featureRotMir = dynamic_cast<SwFilterRotMir *>(&feature);
    VP_PUBLIC_CHK_NULL_RETURN(featureRotMir);

    if (caps.b1stPassOfSfc2PassScaling)
    {
        SwFilterRotMir *filter2ndPass = featureRotMir;
        SwFilterRotMir *filter1ndPass = (SwFilterRotMir *)feature.Clone();
        VP_PUBLIC_CHK_NULL_RETURN(filter1ndPass);
        FeatureParamRotMir &params1stPass = filter1ndPass->GetSwFilterParams();

        // No rotation in 1st pass.
        params1stPass.rotation = VPHAL_ROTATION_IDENTITY;

        // Clear engine caps for filter in 2nd pass.
        filter2ndPass->SetFeatureType(FeatureTypeRotMir);
        filter2ndPass->GetFilterEngineCaps().usedForNextPass = 1;

        executePipe.AddSwFilterUnordered(filter1ndPass, isInputPipe, index);
    }
    else
    {
        return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }

    return MOS_STATUS_SUCCESS;
}
