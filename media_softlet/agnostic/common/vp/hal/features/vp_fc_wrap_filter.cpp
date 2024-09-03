/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_fc_wrap_filter.cpp
//! \brief    Defines the common interface for fc wrap
//!           this file is for the base interface which is shared by all fc wrap in driver.
//!
#include "vp_fc_wrap_filter.h"

namespace vp
{
PolicyFcFeatureWrapHandler::PolicyFcFeatureWrapHandler(VP_HW_CAPS &hwCaps, bool enableL0FC) : PolicyFeatureHandler(hwCaps), m_enableL0FC(enableL0FC)
{
    m_Type = FeatureTypeFc;
    if (m_l0fcFeatureHandler == nullptr)
    {
        m_l0fcFeatureHandler = MOS_New(PolicyL0FcFeatureHandler, hwCaps);
    }
    if (m_fcFeatureHandler == nullptr)
    {
        m_fcFeatureHandler = MOS_New(PolicyFcFeatureHandler, hwCaps);
    }
}

PolicyFcFeatureWrapHandler::~PolicyFcFeatureWrapHandler()
{
    MOS_Delete(m_l0fcFeatureHandler);
    m_l0fcFeatureHandler = nullptr;
    MOS_Delete(m_fcFeatureHandler);
    m_fcFeatureHandler = nullptr;
}

bool PolicyFcFeatureWrapHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    if (m_enableL0FC && !vpExecuteCaps.bFallbackLegacyFC)
    {
        if (!m_l0fcFeatureHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_l0fcFeatureHandler is nullptr");
            return false;
        }
        return m_l0fcFeatureHandler->IsFeatureEnabled(vpExecuteCaps);
    }
    else
    {
        if (!m_fcFeatureHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_fcFeatureHandler is nullptr");
            return false;
        }
        return m_fcFeatureHandler->IsFeatureEnabled(vpExecuteCaps);
    }
}

HwFilterParameter *PolicyFcFeatureWrapHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    if (m_enableL0FC && !vpExecuteCaps.bFallbackLegacyFC)
    {
        if (!m_l0fcFeatureHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_l0fcFeatureHandler is nullptr");
            return nullptr;
        }
        return m_l0fcFeatureHandler->CreateHwFilterParam(vpExecuteCaps, swFilterPipe, pHwInterface);
    }
    else
    {
        if (!m_fcFeatureHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_fcFeatureHandler is nullptr");
            return nullptr;
        }
        return m_fcFeatureHandler->CreateHwFilterParam(vpExecuteCaps, swFilterPipe, pHwInterface);
    }
}

MOS_STATUS PolicyFcFeatureWrapHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    if (m_enableL0FC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_l0fcFeatureHandler);
        return m_l0fcFeatureHandler->UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_fcFeatureHandler);
        return m_fcFeatureHandler->UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
}

MOS_STATUS PolicyFcFeatureWrapHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    if (m_enableL0FC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_l0fcFeatureHandler);
        return m_l0fcFeatureHandler->UpdateUnusedFeature(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_fcFeatureHandler);
        return m_fcFeatureHandler->UpdateUnusedFeature(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
}

MOS_STATUS PolicyFcFeatureWrapHandler::ReleaseHwFeatureParameter(HwFilterParameter *&pParam)
{
    HwFilterFcParameter *fcParam = dynamic_cast<HwFilterFcParameter *>(pParam);
    if (fcParam)
    {
        //this is a legacy FC hw filter param, return it to legacy FC handler
        VP_PUBLIC_CHK_NULL_RETURN(m_fcFeatureHandler);
        return m_fcFeatureHandler->ReleaseHwFeatureParameter(pParam);
    }
    else
    {
        HwFilterL0FcParameter *l0fcParam = dynamic_cast<HwFilterL0FcParameter *>(pParam);
        VP_PUBLIC_CHK_NULL_RETURN(l0fcParam);
        //this is a L0 FC hw filter param, return it to L0 FC handler
        VP_PUBLIC_CHK_NULL_RETURN(m_l0fcFeatureHandler);
        return m_l0fcFeatureHandler->ReleaseHwFeatureParameter(pParam);
    }
}

PolicyFcWrapHandler::PolicyFcWrapHandler(VP_HW_CAPS &hwCaps, bool enableL0FC) : PolicyFeatureHandler(hwCaps), m_enableL0FC(enableL0FC)
{
    m_Type = FeatureTypeFc;
    if (m_l0fcHandler == nullptr)
    {
        m_l0fcHandler = MOS_New(PolicyL0FcHandler, hwCaps);
    }
    if (m_fcHandler == nullptr)
    {
        m_fcHandler = MOS_New(PolicyFcHandler, hwCaps);
    }
}

PolicyFcWrapHandler::~PolicyFcWrapHandler()
{
    MOS_Delete(m_l0fcHandler);
    m_l0fcHandler = nullptr;
    MOS_Delete(m_fcHandler);
    m_fcHandler = nullptr;
}

HwFilterParameter *PolicyFcWrapHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    if (m_enableL0FC && !vpExecuteCaps.bFallbackLegacyFC)
    {
        if (!m_l0fcHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_l0fcHandler is nullptr");
            return nullptr;
        }
        return m_l0fcHandler->CreateHwFilterParam(vpExecuteCaps, swFilterPipe, pHwInterface);
    }
    else
    {
        if (!m_fcHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_fcHandler is nullptr");
            return nullptr;
        }
        return m_fcHandler->CreateHwFilterParam(vpExecuteCaps, swFilterPipe, pHwInterface);
    }
}

bool PolicyFcWrapHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    if (m_enableL0FC && !vpExecuteCaps.bFallbackLegacyFC)
    {
        if (!m_l0fcHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_l0fcHandler is nullptr");
            return false;
        }
        return m_l0fcHandler->IsFeatureEnabled(vpExecuteCaps);
    }
    else
    {
        if (!m_fcHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_fcHandler is nullptr");
            return false;
        }
        return m_fcHandler->IsFeatureEnabled(vpExecuteCaps);
    }
}

MOS_STATUS PolicyFcWrapHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    if (m_enableL0FC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_l0fcHandler);
        return m_l0fcHandler->UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_fcHandler);
        return m_fcHandler->UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
}

MOS_STATUS PolicyFcWrapHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    if (m_enableL0FC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_l0fcHandler);
        return m_l0fcHandler->UpdateUnusedFeature(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_fcHandler);
        return m_fcHandler->UpdateUnusedFeature(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
}

MOS_STATUS PolicyFcWrapHandler::LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps)
{
    if (m_enableL0FC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_l0fcHandler);
        return m_l0fcHandler->LayerSelectForProcess(layerIndexes, featurePipe, caps);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_fcHandler);
        return m_fcHandler->LayerSelectForProcess(layerIndexes, featurePipe, caps);
    }
}

MOS_STATUS PolicyFcWrapHandler::ReleaseHwFeatureParameter(HwFilterParameter *&pParam)
{
    HwFilterFcParameter *fcParam = dynamic_cast<HwFilterFcParameter *>(pParam);
    if (fcParam)
    {
        //this is a legacy FC hw filter param, return it to legacy FC handler
        VP_PUBLIC_CHK_NULL_RETURN(m_fcHandler);
        return m_fcHandler->ReleaseHwFeatureParameter(pParam);
    }
    else
    {
        HwFilterL0FcParameter *l0fcParam = dynamic_cast<HwFilterL0FcParameter *>(pParam);
        VP_PUBLIC_CHK_NULL_RETURN(l0fcParam);
        //this is a L0 FC hw filter param, return it to L0 FC handler
        VP_PUBLIC_CHK_NULL_RETURN(m_l0fcHandler);
        return m_l0fcHandler->ReleaseHwFeatureParameter(pParam);
    }
}

}  // namespace vp