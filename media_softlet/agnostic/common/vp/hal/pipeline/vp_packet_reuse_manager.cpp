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
    VP_PUBLIC_ASSERTMESSAGE("UpdatePacket must be overridden in subclass");
    return MOS_STATUS_INVALID_PARAMETER;
}

MOS_STATUS VpFeatureReuseBase::CheckTeamsParams(bool reusable, bool &reused, SwFilter *filter, uint32_t index)
{
    reused = false;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFeatureReuseBase::StoreTeamsParams(SwFilter *filter, uint32_t index)
{
    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpScalingReuse******************************/
/*******************************************************************/

VpScalingReuse::VpScalingReuse()
{
    m_params_Teams.clear();
}

VpScalingReuse::~VpScalingReuse()
{
}

MOS_STATUS VpScalingReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    VP_FUNC_CALL();
    SwFilterScaling *scaling = dynamic_cast<SwFilterScaling *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(scaling);
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

MOS_STATUS VpScalingReuse::CheckTeamsParams(bool reusable, bool &reused, SwFilter *filter, uint32_t index)
{
    VP_FUNC_CALL();
    SwFilterScaling     *scaling = dynamic_cast<SwFilterScaling *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(scaling);
    FeatureParamScaling &params  = scaling->GetSwFilterParams();
    auto                 it      = m_params_Teams.find(index);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(it, &m_params_Teams);

    if (reusable && params == it->second)
    {
        // No need call UpdateFeatureParams. Just keep compared items updated in m_params
        // is enough. UpdatePacket should use params in swfilter instead of m_params.
        reused = true;
    }
    else
    {
        reused = false;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalingReuse::StoreTeamsParams(SwFilter *filter, uint32_t index)
{
    VP_FUNC_CALL();
    SwFilterScaling     *scaling = dynamic_cast<SwFilterScaling *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(scaling);
    FeatureParamScaling &params  = scaling->GetSwFilterParams();
    auto                 it      = m_params_Teams.find(index);
    if (it != m_params_Teams.end())
    {
        m_params_Teams.erase(index);
    }

    m_params_Teams.insert(std::make_pair(index, params));
    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpCscReuse**********************************/
/*******************************************************************/
VpCscReuse::VpCscReuse()
{
    m_params_Teams.clear();
}

VpCscReuse::~VpCscReuse()
{
}

MOS_STATUS VpCscReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    VP_FUNC_CALL();
    auto IsIefEnabled = [&](PVPHAL_IEF_PARAMS iefParams)
    {
        return (iefParams && iefParams->bEnabled && iefParams->fIEFFactor > 0.0F);
    };

    SwFilterCsc *csc = dynamic_cast<SwFilterCsc *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(csc);
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
    VP_FUNC_CALL();
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
    VP_FUNC_CALL();
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

MOS_STATUS VpCscReuse::CheckTeamsParams(bool reusable, bool &reused, SwFilter *filter, uint32_t index)
{
    VP_FUNC_CALL();
    auto IsIefEnabled = [&](PVPHAL_IEF_PARAMS iefParams) {
        return (iefParams && iefParams->bEnabled && iefParams->fIEFFactor > 0.0F);
    };

    SwFilterCsc     *csc    = dynamic_cast<SwFilterCsc *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(csc);
    FeatureParamCsc &params = csc->GetSwFilterParams();
    auto             it     = m_params_Teams.find(index);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(it, &m_params_Teams);

    // pIEFParams to be updated.
    if (reusable &&
        params.formatInput == it->second.formatInput &&
        params.formatOutput == it->second.formatOutput &&
        params.input == it->second.input &&
        params.output == it->second.output &&
        (nullptr == params.pAlphaParams && nullptr == m_params.pAlphaParams ||
        nullptr != params.pAlphaParams && nullptr != m_params.pAlphaParams &&
        0 == memcmp(params.pAlphaParams, m_params.pAlphaParams, sizeof(VPHAL_ALPHA_PARAMS))) &&
        IsIefEnabled(params.pIEFParams) == false)
    {
        reused = true;
    }
    else
    {
        reused = false;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscReuse::StoreTeamsParams(SwFilter *filter, uint32_t index)
{
    VP_FUNC_CALL();
    SwFilterCsc     *csc    = dynamic_cast<SwFilterCsc *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(csc);
    FeatureParamCsc &params = csc->GetSwFilterParams();
    auto             it     = m_params_Teams.find(index);
    if (it != m_params_Teams.end())
    {
        m_params_Teams.erase(index);
    }

    m_params_Teams.insert(std::make_pair(index, params));
    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpRotMirReuse*******************************/
/*******************************************************************/

VpRotMirReuse::VpRotMirReuse()
{
    m_params_Teams.clear();
}

VpRotMirReuse::~VpRotMirReuse()
{
}

MOS_STATUS VpRotMirReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    VP_FUNC_CALL();
    SwFilterRotMir *rot = dynamic_cast<SwFilterRotMir *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(rot);
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

MOS_STATUS VpRotMirReuse::CheckTeamsParams(bool reusable, bool &reused, SwFilter *filter, uint32_t index)
{
    VP_FUNC_CALL();

    SwFilterRotMir     *rot    = dynamic_cast<SwFilterRotMir *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(rot);

    FeatureParamRotMir &params = rot->GetSwFilterParams();
    auto               it      = m_params_Teams.find(index);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(it, &m_params_Teams);

    // pIEFParams to be updated.
    if (reusable &&
        params == it->second)
    {
        reused = true;
    }
    else
    {
        reused = false;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRotMirReuse::StoreTeamsParams(SwFilter *filter, uint32_t index)
{
    VP_FUNC_CALL();
    SwFilterRotMir     *rot    = dynamic_cast<SwFilterRotMir *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(rot);

    FeatureParamRotMir &params = rot->GetSwFilterParams();
    auto                it     = m_params_Teams.find(index);
    if (it != m_params_Teams.end())
    {
        m_params_Teams.erase(index);
    }

    m_params_Teams.insert(std::make_pair(index, params));
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
    VP_FUNC_CALL();
    SwFilterColorFill *colorfill = dynamic_cast<SwFilterColorFill *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(colorfill);
    FeatureParamColorFill &params = colorfill->GetSwFilterParams();
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
    VP_FUNC_CALL();
    SwFilterAlpha *alpha = dynamic_cast<SwFilterAlpha *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(alpha);
    FeatureParamAlpha &params = alpha->GetSwFilterParams();
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
    VP_FUNC_CALL();
    SwFilterDenoise     *dn    = dynamic_cast<SwFilterDenoise *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(dn);
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
    VP_FUNC_CALL();
    VpVeboxCmdPacketBase *veboxPacket = dynamic_cast<VpVeboxCmdPacketBase *>(packet);
    VP_PUBLIC_CHK_NULL_RETURN(veboxPacket);

    SwFilterDenoise *denoise = dynamic_cast<SwFilterDenoise *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(denoise);
    FeatureParamDenoise &params = denoise->GetSwFilterParams();

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
    VP_FUNC_CALL();
    SwFilterTcc *tcc = dynamic_cast<SwFilterTcc *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(tcc);
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
    VP_FUNC_CALL();
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
    VP_FUNC_CALL();
    SwFilterSte     *ste    = dynamic_cast<SwFilterSte *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(ste);
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
    VP_FUNC_CALL();
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
/***********************VpProcampReuse**********************************/
/*******************************************************************/

VpProcampReuse::VpProcampReuse()
{
}

VpProcampReuse::~VpProcampReuse()
{
}

MOS_STATUS VpProcampReuse::UpdateFeatureParams(bool reusable, bool &reused, SwFilter *filter)
{
    VP_FUNC_CALL();
    SwFilterProcamp *procamp    = dynamic_cast<SwFilterProcamp *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(procamp);
    FeatureParamProcamp &params  = procamp->GetSwFilterParams();
    if (reusable && ((params.procampParams && m_params.procampParams && params.procampParams->bEnabled == m_params.procampParams->bEnabled) || 
        (!params.procampParams && !m_params.procampParams)))
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

MOS_STATUS VpProcampReuse::UpdatePacket(SwFilter *filter, VpCmdPacket *packet)
{
    VP_FUNC_CALL();
    VpVeboxCmdPacketBase *veboxPacket = dynamic_cast<VpVeboxCmdPacketBase *>(packet);
    VP_PUBLIC_CHK_NULL_RETURN(veboxPacket);

    SwFilterProcamp *procamp = dynamic_cast<SwFilterProcamp *>(filter);
    VP_PUBLIC_CHK_NULL_RETURN(procamp);
    FeatureParamProcamp &params = procamp->GetSwFilterParams();

    VP_PUBLIC_CHK_STATUS_RETURN(veboxPacket->UpdateProcampParams(params));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpProcampReuse::UpdateFeatureParams(FeatureParamProcamp &params)
{
    m_params = params;
    if (params.procampParams)
    {
        m_procampParams        = *params.procampParams;
        m_params.procampParams = &m_procampParams;
    }
    return MOS_STATUS_SUCCESS;
}

/*******************************************************************/
/***********************VpPacketReuseManager************************/
/*******************************************************************/

VpPacketReuseManager::VpPacketReuseManager(PacketPipeFactory &packetPipeFactory, VpUserFeatureControl &userFeatureControl) :
    m_packetPipeFactory(packetPipeFactory), m_disablePacketReuse(userFeatureControl.IsPacketReuseDisabled())
{
    m_pipeReused_TeamsPacket.clear();
    m_enablePacketReuseTeamsAlways = userFeatureControl.IsPacketReuseEnabledTeamsAlways();
}

VpPacketReuseManager::~VpPacketReuseManager()
{
    for (uint32_t index = 0; index < m_pipeReused_TeamsPacket.size(); index++)
    {
        auto pipeReuseHandle = m_pipeReused_TeamsPacket.find(index);
        if (pipeReuseHandle != m_pipeReused_TeamsPacket.end() &&
            pipeReuseHandle->second != m_pipeReused)
        {
            m_packetPipeFactory.ReturnPacketPipe(pipeReuseHandle->second);
        }
    }
    m_pipeReused_TeamsPacket.clear();

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

    p = MOS_New(VpProcampReuse);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_features.insert(std::make_pair(FeatureTypeProcamp, p));


    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPacketReuseManager::PreparePacketPipeReuse(SwFilterPipe *&swFilterPipe, Policy &policy, VpResourceManager &resMgr, bool &isPacketPipeReused, bool &isTeamsWL)
{
    VP_FUNC_CALL();
    bool reusableOfLastPipe = m_reusable;
    bool foundPipe          = false;
    uint32_t index          = 0;

    m_reusable = true;

    if (m_disablePacketReuse)
    {
        m_reusable = false;
        VP_PUBLIC_NORMALMESSAGE("Not reusable since Packet reuse disabled.");
        return MOS_STATUS_SUCCESS;
    }

    if (nullptr == swFilterPipe || swFilterPipe->GetSurfaceCount(true) != 1)
    {
        m_reusable = false;
        VP_PUBLIC_NORMALMESSAGE("Not reusable for multi-layer cases.");

        if (m_pipeReused)
        {
            ReturnPacketPipeReused();
        }

        return MOS_STATUS_SUCCESS;
    }

    auto &pipe = *swFilterPipe;
    auto featureRegistered = policy.GetFeatureRegistered();

    isPacketPipeReused = true;

    for (auto feature : featureRegistered)
    {
        SwFilter *swfilter = pipe.GetSwFilter(true, 0, feature);
        auto it = m_features.find(feature);
        bool ignoreUpdateFeatureParams = false;

        if (m_features.end() == it)
        {
            if (nullptr != swfilter)
            {
                // unreused feature && nullptr != swfilter
                m_reusable         = false;
                isPacketPipeReused = false;
                if (m_pipeReused)
                {
                    ReturnPacketPipeReused();
                }
                return MOS_STATUS_SUCCESS;
            }
            else
            {
                // unreused feature && nullptr == swfilter
                continue;
            }
        }
        else
        {
            // handle reused feature
            VP_PUBLIC_CHK_STATUS_RETURN(it->second->HandleNullSwFilter(reusableOfLastPipe, isPacketPipeReused, swfilter, ignoreUpdateFeatureParams));
            if (ignoreUpdateFeatureParams)
            {
                continue;
            }
        }

        bool reused = false;
        VP_PUBLIC_CHK_STATUS_RETURN(it->second->UpdateFeatureParams(reusableOfLastPipe, reused, swfilter));
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

    m_TeamsPacket       = false;
    m_TeamsPacket_reuse = false;

    if (isTeamsWL || m_enablePacketReuseTeamsAlways)
    {
        for (auto feature : featureRegistered)
        {
            SwFilter *swfilter = pipe.GetSwFilter(true, 0, feature);
            if (nullptr == swfilter)
            {
                continue;
            }

            if (feature == FeatureTypeCsc ||
                feature == FeatureTypeScaling ||
                feature == FeatureTypeRotMir)
            {
                // Teams feature
            }
            else
            {
                m_TeamsPacket = false;
                break;
            }

            m_TeamsPacket = true;
        }

        if (nullptr == swFilterPipe || swFilterPipe->GetSurfaceCount(true) != 1 || swFilterPipe->GetSurfaceCount(false) != 1)
        {
            m_TeamsPacket = false;
        }
    }

    if (!isPacketPipeReused && m_TeamsPacket)
    {
        SwFilter *scaling = pipe.GetSwFilter(true, 0, FeatureTypeScaling);
        SwFilter *csc     = pipe.GetSwFilter(true, 0, FeatureTypeCsc);
        SwFilter *rot     = pipe.GetSwFilter(true, 0, FeatureTypeRotMir);

        bool reused = false;

        auto scalingreuse = m_features.find(FeatureTypeScaling);
        auto cscreuse     = m_features.find(FeatureTypeCsc);
        auto rotreuse     = m_features.find(FeatureTypeRotMir);
        if (scalingreuse == m_features.end() ||
            cscreuse == m_features.end()     ||
            rotreuse == m_features.end())
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_HANDLE);
        }

        for (index = 0; index < m_pipeReused_TeamsPacket.size(); index++)
        {
            scalingreuse->second->CheckTeamsParams(reusableOfLastPipe, reused, scaling, index);
            if (!reused)
            {
                continue;
            }

            cscreuse->second->CheckTeamsParams(reusableOfLastPipe, reused, csc, index);
            if (!reused)
            {
                continue;
            }

            rotreuse->second->CheckTeamsParams(reusableOfLastPipe, reused, rot, index);
            if (reused)
            {
                break;
            }
        }
        // if not found, store the new params and packet
        if (!reused)
        {
            scalingreuse->second->StoreTeamsParams(scaling, curIndex);
            cscreuse->second->StoreTeamsParams(csc, curIndex);
            rotreuse->second->StoreTeamsParams(rot, curIndex);

            m_TeamsPacket_reuse = false;

            ReturnPacketPipeReused();

            return MOS_STATUS_SUCCESS;
        }
        else
        {
            auto pipe_TeamsPacket = m_pipeReused_TeamsPacket.find(index);

            VpCmdPacket *packet = pipe_TeamsPacket->second->GetPacket(0);
            VP_PUBLIC_CHK_NULL_RETURN(packet);


            m_pipeReused = pipe_TeamsPacket->second;

            VP_SURFACE_SETTING surfSetting = {};
            VP_EXECUTE_CAPS    caps        = packet->GetExecuteCaps();
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

            m_TeamsPacket_reuse = true;
            isPacketPipeReused  = true;
            return MOS_STATUS_SUCCESS;
        }
    }

    if (!isPacketPipeReused)
    {
        // m_pipeReused will be udpated in UpdatePacketPipeConfig.
        VP_PUBLIC_NORMALMESSAGE("Packet cannot be reused.");

        ReturnPacketPipeReused();

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

    if (caps.enableSFCLinearOutputByTileConvert)
    {
        VP_PUBLIC_NORMALMESSAGE("Not reusable for enableSFCLinearOutputByTileConvert case.");
        m_reusable = false;
        return MOS_STATUS_SUCCESS;
    }

    if (m_TeamsPacket && !m_TeamsPacket_reuse)
    {
        auto it = m_pipeReused_TeamsPacket.find(curIndex);
        if (it != m_pipeReused_TeamsPacket.end())
        {
            m_packetPipeFactory.ReturnPacketPipe(it->second);
            m_pipeReused_TeamsPacket.erase(curIndex);
        }

        m_pipeReused_TeamsPacket.insert(std::make_pair(curIndex, pipe));

        curIndex++;
        if (curIndex >= MaxTeamsPacketSize)
        {
            curIndex = 0;
        }
    }

    if (!m_TeamsPacket)
    {
        ReturnPacketPipeReused();
    }

    m_pipeReused = pipe;

    pipe = nullptr;

    return MOS_STATUS_SUCCESS;
}

void VpPacketReuseManager::ReturnPacketPipeReused()
{
    VP_FUNC_CALL();
    if (nullptr == m_pipeReused)
    {
        return;
    }
    for (const auto &pair : m_pipeReused_TeamsPacket)
    {
        if (pair.second == m_pipeReused)
        {
            m_pipeReused = nullptr;
            return;
        }
    }
    m_packetPipeFactory.ReturnPacketPipe(m_pipeReused);
    return;
}
