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
//! \file     vp_packet_reuse_manager.cpp
//! \brief    Defines the common classes for vp packet reuse
//!           this file is for the base interface which is shared by all features.
//!

#include "vp_packet_reuse_manager.h"
#include "vp_vebox_cmd_packet_base.h"
#include "vp_user_feature_control.h"
#include "policy.h"

using namespace vp;

/*******************************************************************/
/***********************VpFeatureReuseBase**************************/
/*******************************************************************/

VpFeatureReuseBase::~VpFeatureReuseBase()
{
}

MOS_STATUS VpFeatureReuseBase::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    reused = false;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFeatureReuseBase::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpScalingReuse******************************/
/*******************************************************************/

VpScalingReuse::VpScalingReuse()
{
}

VpScalingReuse::~VpScalingReuse()
{
}

MOS_STATUS VpScalingReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(filter);
    FeatureParamScaling &params = scaling->GetSwFilterParams();
    if (reusable && params == m_params)
    {
        // No need call UpdateFeatureParams. Just keep compared items updated in m_params
        // is enough. UpdatePacket should use params in swfilter instead of m_params.
        reused = true;
    }
    else
    {
        reused = false;
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureParams(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingReuse::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    // Parameters should be no change in UpdateFeatureParams.
    // No need update.
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingReuse::UpdateFeatureParams(FeatureParamScaling &params)
{
    m_params = params;
    if (params.pColorFillParams)
    {
        m_colorFillParams = *params.pColorFillParams;
        m_params.pColorFillParams = &m_colorFillParams;
    }
    if (params.pCompAlpha)
    {
        m_compAlpha = *params.pCompAlpha;
        m_params.pCompAlpha = &m_compAlpha;
    }
    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpCscReuse**********************************/
/*******************************************************************/
VpCscReuse::VpCscReuse()
{
}

VpCscReuse::~VpCscReuse()
{
}

MOS_STATUS VpCscReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    auto IsIefEnabled = [&](PVPHAL_IEF_PARAMS iefParams)
    {
        return (iefParams && iefParams->bEnabled && iefParams->fIEFFactor > 0.0F);
    };

    SwFilterCsc *csc = dynamic_cast<SwFilterCsc *>(filter);
    FeatureParamCsc &params = csc->GetSwFilterParams();
    // pIEFParams to be updated.
    if (reusable &&
        params.formatInput == m_params.formatInput &&
        params.formatOutput == m_params.formatOutput &&
        params.input == m_params.input &&
        params.output == m_params.output &&
        (nullptr == params.pAlphaParams && nullptr == m_params.pAlphaParams ||
        nullptr != params.pAlphaParams && nullptr != m_params.pAlphaParams &&
        0 == memcmp(params.pAlphaParams, m_params.pAlphaParams, sizeof(VPHAL_ALPHA_PARAMS))) &&
        IsIefEnabled(params.pIEFParams) == IsIefEnabled(m_params.pIEFParams))
    {
        // No need call UpdateFeatureParams. Just keep compared items updated in m_params
        // is enough. UpdatePacket should use params in swfilter instead of m_params.
        reused = true;
    }
    else
    {
        reused = false;
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureParams(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscReuse::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    VpVeboxCmdPacketBase *veboxPacket = dynamic_cast<VpVeboxCmdPacketBase *>(packet);
    VP_PUBLIC_CHK_NULL_RETURN(veboxPacket);

    SwFilterCsc *scaling = dynamic_cast<SwFilterCsc *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(scaling);
    FeatureParamCsc &params = scaling->GetSwFilterParams();

    VP_PUBLIC_CHK_STATUS_RETURN(veboxPacket->UpdateCscParams(params));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscReuse::UpdateFeatureParams(FeatureParamCsc &params)
{
    m_params = params;

    if (params.pAlphaParams)
    {
        m_alphaParams = *params.pAlphaParams;
        m_params.pAlphaParams = &m_alphaParams;
    }
    else
    {
        m_params.pAlphaParams = nullptr;
    }

    if (params.pIEFParams)
    {
        m_iefParams = *params.pIEFParams;
        m_params.pIEFParams = &m_iefParams;
    }
    else
    {
        m_params.pIEFParams = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpRotMirReuse*******************************/
/*******************************************************************/

VpRotMirReuse::VpRotMirReuse()
{
}

VpRotMirReuse::~VpRotMirReuse()
{
}

MOS_STATUS VpRotMirReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    SwFilterRotMir *rot = dynamic_cast<SwFilterRotMir *>(filter);
    FeatureParamRotMir &params = rot->GetSwFilterParams();
    if (reusable && params == m_params)
    {
        // No need call UpdateFeatureParams. Just keep compared items updated in m_params
        // is enough. UpdatePacket should use params in swfilter instead of m_params.
        reused = true;
    }
    else
    {
        reused = false;
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureParams(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRotMirReuse::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    // Parameters should be no change in UpdateFeatureParams.
    // No need update.
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRotMirReuse::UpdateFeatureParams(FeatureParamRotMir &params)
{
    m_params = params;

    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpColorFillReuse****************************/
/*******************************************************************/

VpColorFillReuse::VpColorFillReuse()
{
}

VpColorFillReuse::~VpColorFillReuse()
{
}

MOS_STATUS VpColorFillReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    SwFilterColorFill *rot = dynamic_cast<SwFilterColorFill *>(filter);
    FeatureParamColorFill &params = rot->GetSwFilterParams();
    if (reusable && params == m_params)
    {
        // No need call UpdateFeatureParams. Just keep compared items updated in m_params
        // is enough. UpdatePacket should use params in swfilter instead of m_params.
        reused = true;
    }
    else
    {
        reused = false;
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureParams(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpColorFillReuse::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    // Parameters should be no change in UpdateFeatureParams.
    // No need update.
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpColorFillReuse::UpdateFeatureParams(FeatureParamColorFill &params)
{
    m_params = params;

    if (params.colorFillParams)
    {
        m_colorFillParams = *params.colorFillParams;
        m_params.colorFillParams = &m_colorFillParams;
    }

    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpAlphaReuse********************************/
/*******************************************************************/

VpAlphaReuse::VpAlphaReuse()
{
}

VpAlphaReuse::~VpAlphaReuse()
{
}

MOS_STATUS VpAlphaReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    SwFilterAlpha *rot = dynamic_cast<SwFilterAlpha *>(filter);
    FeatureParamAlpha &params = rot->GetSwFilterParams();
    if (reusable && params == m_params)
    {
        // No need call UpdateFeatureParams. Just keep compared items updated in m_params
        // is enough. UpdatePacket should use params in swfilter instead of m_params.
        reused = true;
    }
    else
    {
        reused = false;
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureParams(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAlphaReuse::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    // Parameters should be no change in UpdateFeatureParams.
    // No need update.
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAlphaReuse::UpdateFeatureParams(FeatureParamAlpha &params)
{
    m_params = params;

    if (params.compAlpha)
    {
        m_compAlpha = *params.compAlpha;
        m_params.compAlpha = &m_compAlpha;
    }

    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpDenoiseReuse********************************/
/*******************************************************************/

VpDenoiseReuse::VpDenoiseReuse()
{
}

VpDenoiseReuse::~VpDenoiseReuse()
{
}

MOS_STATUS VpDenoiseReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    SwFilterDenoise     *dn    = dynamic_cast<SwFilterDenoise *>(filter);
    FeatureParamDenoise &params = dn->GetSwFilterParams();
    if (reusable && params == m_params)
    {
        // No need call UpdateFeatureParams. Just keep compared items updated in m_params
        // is enough. UpdatePacket should use params in swfilter instead of m_params.
        reused = true;
    }
    else
    {
        reused = false;
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureParams(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpDenoiseReuse::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    VpVeboxCmdPacketBase *veboxPacket = dynamic_cast<VpVeboxCmdPacketBase *>(packet);
    VP_PUBLIC_CHK_NULL_RETURN(veboxPacket);

    SwFilterDenoise *scaling = dynamic_cast<SwFilterDenoise *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(scaling);
    FeatureParamDenoise &params = scaling->GetSwFilterParams();

    VP_PUBLIC_CHK_STATUS_RETURN(veboxPacket->UpdateDenoiseParams(params));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpDenoiseReuse::UpdateFeatureParams(FeatureParamDenoise &params)
{
    m_params = params;
    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpTccReuse**********************************/
/*******************************************************************/

VpTccReuse::VpTccReuse()
{
}

VpTccReuse::~VpTccReuse()
{
}

MOS_STATUS VpTccReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    SwFilterTcc *tcc = dynamic_cast<SwFilterTcc *>(filter);
    FeatureParamTcc &params = tcc->GetSwFilterParams();
    if (reusable && params.bEnableTCC == m_params.bEnableTCC)
    {
        // No need call UpdateFeatureParams. Just keep compared items updated in m_params
        // is enough. UpdatePacket should use params in swfilter instead of m_params.
        reused = true;
    }
    else
    {
        reused = false;
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureParams(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpTccReuse::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    VpVeboxCmdPacketBase *veboxPacket = dynamic_cast<VpVeboxCmdPacketBase *>(packet);
    VP_PUBLIC_CHK_NULL_RETURN(veboxPacket);

    SwFilterTcc *tcc = dynamic_cast<SwFilterTcc *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(tcc);
    FeatureParamTcc &params = tcc->GetSwFilterParams();

    VP_PUBLIC_CHK_STATUS_RETURN(veboxPacket->UpdateTccParams(params));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpTccReuse::UpdateFeatureParams(FeatureParamTcc &params)
{
    m_params = params;
    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpSteReuse**********************************/
/*******************************************************************/

VpSteReuse::VpSteReuse()
{
}

VpSteReuse::~VpSteReuse()
{
}

MOS_STATUS VpSteReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    SwFilterSte     *ste    = dynamic_cast<SwFilterSte *>(filter);
    FeatureParamSte &params = ste->GetSwFilterParams();
    if (reusable && params.bEnableSTE == m_params.bEnableSTE)
    {
        // No need call UpdateFeatureParams. Just keep compared items updated in m_params
        // is enough. UpdatePacket should use params in swfilter instead of m_params.
        reused = true;
    }
    else
    {
        reused = false;
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureParams(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpSteReuse::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    VpVeboxCmdPacketBase *veboxPacket = dynamic_cast<VpVeboxCmdPacketBase *>(packet);
    VP_PUBLIC_CHK_NULL_RETURN(veboxPacket);

    SwFilterSte *ste = dynamic_cast<SwFilterSte *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(ste);
    FeatureParamSte &params = ste->GetSwFilterParams();

    VP_PUBLIC_CHK_STATUS_RETURN(veboxPacket->UpdateSteParams(params));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpSteReuse::UpdateFeatureParams(FeatureParamSte &params)
{
    m_params = params;
    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpPacketReuseManager************************/
/*******************************************************************/

VpPacketReuseManager::VpPacketReuseManager(PacketPipeFactory &packetPipeFactory, VpUserFeatureControl &userFeatureControl) :
    m_packetPipeFactory(packetPipeFactory), m_disablePacketReuse(userFeatureControl.IsPacketReuseDisabled())
{
}

VpPacketReuseManager::~VpPacketReuseManager()
{
    if (m_pipeReused)
    {
        m_packetPipeFactory.ReturnPacketPipe(m_pipeReused);
    }
    for (auto &it : m_features)
    {
        if (it.second)
        {
            MOS_Delete(it.second);
        }
    }
    m_features.clear();
}

MOS_STATUS VpPacketReuseManager::RegisterFeatures()
{
    VP_FUNC_CALL()
    if (m_disablePacketReuse)
    {
        return MOS_STATUS_SUCCESS;
    }
    VpFeatureReuseBase *p = MOS_New(VpScalingReuse);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_features.insert(std::make_pair(FeatureTypeScaling, p));

    p = MOS_New(VpCscReuse);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_features.insert(std::make_pair(FeatureTypeCsc, p));

    p = MOS_New(VpRotMirReuse);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_features.insert(std::make_pair(FeatureTypeRotMir, p));

    p = MOS_New(VpColorFillReuse);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_features.insert(std::make_pair(FeatureTypeColorFill, p));

    p = MOS_New(VpDenoiseReuse);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_features.insert(std::make_pair(FeatureTypeDn, p));

    p = MOS_New(VpAlphaReuse);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_features.insert(std::make_pair(FeatureTypeAlpha, p));

    p = MOS_New(VpTccReuse);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_features.insert(std::make_pair(FeatureTypeTcc, p));

    p = MOS_New(VpSteReuse);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_features.insert(std::make_pair(FeatureTypeSte, p));


    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPacketReuseManager::PreparePacketPipeReuse(std::vector<SwFilterPipe*> &swFilterPipes, Policy &policy, VpResourceManager &resMgr, bool &isPacketPipeReused)
{
    VP_FUNC_CALL();
    bool reusableOfLastPipe = m_reusable;
    m_reusable = true;

    if (m_disablePacketReuse)
    {
        m_reusable = false;
        VP_PUBLIC_NORMALMESSAGE("Not reusable since Packet reuse disabled.");
        return MOS_STATUS_SUCCESS;
    }

    if (swFilterPipes.size() != 1 || nullptr == swFilterPipes[0] ||
        swFilterPipes[0]->GetSurfaceCount(true) != 1)
    {
        m_reusable = false;
        VP_PUBLIC_NORMALMESSAGE("Not reusable for multi-layer cases.");

        if (m_pipeReused)
        {
            m_packetPipeFactory.ReturnPacketPipe(m_pipeReused);
        }

        return MOS_STATUS_SUCCESS;
    }

    auto &pipe = *swFilterPipes[0];
    auto featureRegistered = policy.GetFeatureRegistered();

    isPacketPipeReused = true;

    for (auto feature : featureRegistered)
    {
        SwFilter *swfilter = pipe.GetSwFilter(true, 0, feature);
        if (nullptr == swfilter)
        {
            continue;
        }
        auto it = m_features.find(feature);
        if (m_features.end() == it)
        {
            m_reusable = false;
            return MOS_STATUS_SUCCESS;
        }
        bool reused = false;
        it->second->UpdateFeatureParams(reusableOfLastPipe, reused, swfilter);
        if (!reused)
        {
            VP_PUBLIC_NORMALMESSAGE("Packet not reused for feature %d", feature);
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("Packet  reused for feature %d", feature);
        }
        isPacketPipeReused &= reused;
    }

    if (!isPacketPipeReused)
    {
        // m_pipeReused will be udpated in UpdatePacketPipeConfig.
        VP_PUBLIC_NORMALMESSAGE("Packet cannot be reused.");

        if (m_pipeReused)
        {
            m_packetPipeFactory.ReturnPacketPipe(m_pipeReused);
        }

        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_CHK_NULL_RETURN(m_pipeReused);

    if (0 == m_pipeReused->PacketNum())
    {
        VP_PUBLIC_ASSERTMESSAGE("Invalid pipe for reuse!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    VpCmdPacket *packet = m_pipeReused->GetPacket(0);
    VP_PUBLIC_CHK_NULL_RETURN(packet);

    VP_SURFACE_SETTING surfSetting = {};
    VP_EXECUTE_CAPS caps = packet->GetExecuteCaps();
    resMgr.GetUpdatedExecuteResource(featureRegistered, caps, pipe, surfSetting);

    VP_PUBLIC_CHK_STATUS_RETURN(packet->PacketInitForReuse(pipe.GetSurface(true, 0), pipe.GetSurface(false, 0), pipe.GetPastSurface(0), surfSetting, caps));

    // Update Packet
    for (auto it : m_features)
    {
        SwFilter *swfilter = pipe.GetSwFilter(true, 0, it.first);
        if (nullptr == swfilter)
        {
            continue;
        }

        VP_PUBLIC_NORMALMESSAGE("Update Packet for feature %d", it.first);
        VP_PUBLIC_CHK_STATUS_RETURN(it.second->UpdatePacket(swfilter, packet));
    }

    return MOS_STATUS_SUCCESS;
}

// Be called for not reused case before packet pipe execution.
MOS_STATUS VpPacketReuseManager::UpdatePacketPipeConfig(PacketPipe *&pipe)
{
    VP_FUNC_CALL();
    if (!m_reusable)
    {
        VP_PUBLIC_NORMALMESSAGE("Bypass UpdatePacketPipeConfig since not reusable.");
        return MOS_STATUS_SUCCESS;
    }
    if (nullptr == pipe || pipe->PacketNum() > 1)
    {
        VP_PUBLIC_NORMALMESSAGE("Not reusable for multi-pass case.");
        m_reusable = false;
        return MOS_STATUS_SUCCESS;
    }

    auto *packet = pipe->GetPacket(0);
    if (nullptr == packet)
    {
        VP_PUBLIC_ASSERTMESSAGE("Invalid packet!");
        m_reusable = false;
        VP_PUBLIC_CHK_NULL_RETURN(packet);
    }

    VP_EXECUTE_CAPS caps = packet->GetExecuteCaps();
    if (caps.bRender)
    {
        VP_PUBLIC_NORMALMESSAGE("Not reusable for render case.");
        m_reusable = false;
        return MOS_STATUS_SUCCESS;
    }

    if (m_pipeReused)
    {
        m_packetPipeFactory.ReturnPacketPipe(m_pipeReused);
    }

    m_pipeReused = pipe;
    pipe = nullptr;

    return MOS_STATUS_SUCCESS;
}

