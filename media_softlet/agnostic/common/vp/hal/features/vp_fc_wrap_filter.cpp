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
PolicyFcFeatureWrapHandler::PolicyFcFeatureWrapHandler(VP_HW_CAPS &hwCaps, bool enableOclFC) : PolicyFeatureHandler(hwCaps), m_enableOclFC(enableOclFC)
{
    m_Type = FeatureTypeFc;
    if (m_oclfcFeatureHandler == nullptr)
    {
        m_oclfcFeatureHandler = MOS_New(PolicyOclFcFeatureHandler, hwCaps);
    }
    if (m_fcFeatureHandler == nullptr)
    {
        m_fcFeatureHandler = MOS_New(PolicyFcFeatureHandler, hwCaps);
    }
}

PolicyFcFeatureWrapHandler::~PolicyFcFeatureWrapHandler()
{
    MOS_Delete(m_oclfcFeatureHandler);
    m_oclfcFeatureHandler = nullptr;
    MOS_Delete(m_fcFeatureHandler);
    m_fcFeatureHandler = nullptr;
}

bool PolicyFcFeatureWrapHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    if (m_enableOclFC && !vpExecuteCaps.bFallbackLegacyFC)
    {
        if (!m_oclfcFeatureHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_oclfcFeatureHandler is nullptr");
            return false;
        }
        return m_oclfcFeatureHandler->IsFeatureEnabled(vpExecuteCaps);
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
    if (m_enableOclFC && !vpExecuteCaps.bFallbackLegacyFC)
    {
        if (!m_oclfcFeatureHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_oclfcFeatureHandler is nullptr");
            return nullptr;
        }
        return m_oclfcFeatureHandler->CreateHwFilterParam(vpExecuteCaps, swFilterPipe, pHwInterface);
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
    if (m_enableOclFC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_oclfcFeatureHandler);
        return m_oclfcFeatureHandler->UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_fcFeatureHandler);
        return m_fcFeatureHandler->UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
}

MOS_STATUS PolicyFcFeatureWrapHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    if (m_enableOclFC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_oclfcFeatureHandler);
        return m_oclfcFeatureHandler->UpdateUnusedFeature(caps, feature, featurePipe, executePipe, isInputPipe, index);
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
        HwFilterOclFcParameter *oclfcParam = dynamic_cast<HwFilterOclFcParameter *>(pParam);
        VP_PUBLIC_CHK_NULL_RETURN(oclfcParam);
        //this is a OCL FC hw filter param, return it to OCL FC handler
        VP_PUBLIC_CHK_NULL_RETURN(m_oclfcFeatureHandler);
        return m_oclfcFeatureHandler->ReleaseHwFeatureParameter(pParam);
    }
}

PolicyFcWrapHandler::PolicyFcWrapHandler(VP_HW_CAPS &hwCaps, bool enableOclFC) : PolicyFeatureHandler(hwCaps), m_enableOclFC(enableOclFC)
{
    m_Type = FeatureTypeFc;
    if (m_oclfcHandler == nullptr)
    {
        m_oclfcHandler = MOS_New(PolicyOclFcHandler, hwCaps);
    }
    if (m_fcHandler == nullptr)
    {
        m_fcHandler = MOS_New(PolicyFcHandler, hwCaps);
    }
}

PolicyFcWrapHandler::~PolicyFcWrapHandler()
{
    MOS_Delete(m_oclfcHandler);
    m_oclfcHandler = nullptr;
    MOS_Delete(m_fcHandler);
    m_fcHandler = nullptr;
}

HwFilterParameter *PolicyFcWrapHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    if (m_enableOclFC && !vpExecuteCaps.bFallbackLegacyFC)
    {
        if (!m_oclfcHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_oclfcHandler is nullptr");
            return nullptr;
        }
        return m_oclfcHandler->CreateHwFilterParam(vpExecuteCaps, swFilterPipe, pHwInterface);
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
    if (m_enableOclFC && !vpExecuteCaps.bFallbackLegacyFC)
    {
        if (!m_oclfcHandler)
        {
            VP_PUBLIC_ASSERTMESSAGE("m_oclfcHandler is nullptr");
            return false;
        }
        return m_oclfcHandler->IsFeatureEnabled(vpExecuteCaps);
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
    if (m_enableOclFC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_oclfcHandler);
        return m_oclfcHandler->UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_fcHandler);
        return m_fcHandler->UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
}

MOS_STATUS PolicyFcWrapHandler::UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    if (m_enableOclFC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_oclfcHandler);
        return m_oclfcHandler->UpdateUnusedFeature(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_fcHandler);
        return m_fcHandler->UpdateUnusedFeature(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }
}

MOS_STATUS PolicyFcWrapHandler::LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps)
{
    if (m_enableOclFC && !caps.bFallbackLegacyFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_oclfcHandler);
        return m_oclfcHandler->LayerSelectForProcess(layerIndexes, featurePipe, caps);
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
        HwFilterOclFcParameter *oclfcParam = dynamic_cast<HwFilterOclFcParameter *>(pParam);
        VP_PUBLIC_CHK_NULL_RETURN(oclfcParam);
        //this is a OCL FC hw filter param, return it to OCL FC handler
        VP_PUBLIC_CHK_NULL_RETURN(m_oclfcHandler);
        return m_oclfcHandler->ReleaseHwFeatureParameter(pParam);
    }
}

}  // namespace vp