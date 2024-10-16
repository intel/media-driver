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
//! \file     vp_fc_wrap_filter.h
//! \brief    Defines the common interface for legacy fc and OCL fc wrapper
//!           this file is for the base interface which is shared by all wrap fc in driver.
#ifndef __VP_FC_WRAP_FILTER_H__
#define __VP_FC_WRAP_FILTER_H__

#include "vp_ocl_fc_filter.h"
#include "vp_fc_filter.h"

namespace vp
{

class PolicyFcFeatureWrapHandler : public PolicyFeatureHandler
{
public:
    PolicyFcFeatureWrapHandler(VP_HW_CAPS &hwCaps, bool enableOclFC);

    virtual ~PolicyFcFeatureWrapHandler();

    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);

    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    virtual MOS_STATUS UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);

    virtual MOS_STATUS UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);

    virtual MOS_STATUS ReleaseHwFeatureParameter(HwFilterParameter *&pParam) override;

private:
    bool                       m_enableOclFC         = false;
    PolicyOclFcFeatureHandler *m_oclfcFeatureHandler = nullptr;
    PolicyFcFeatureHandler    *m_fcFeatureHandler    = nullptr;

    MEDIA_CLASS_DEFINE_END(vp__PolicyFcFeatureWrapHandler)
};

class PolicyFcWrapHandler : public PolicyFeatureHandler
{
public:
    PolicyFcWrapHandler(VP_HW_CAPS &hwCaps, bool enableOclFC);

    virtual ~PolicyFcWrapHandler();

    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface) override;

    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);

    virtual MOS_STATUS UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);

    virtual MOS_STATUS UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);

    virtual MOS_STATUS LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps);

    virtual MOS_STATUS ReleaseHwFeatureParameter(HwFilterParameter *&pParam) override;

private:
    bool                m_enableOclFC   = false;
    PolicyOclFcHandler *m_oclfcHandler = nullptr;
    PolicyFcHandler    *m_fcHandler    = nullptr;

    MEDIA_CLASS_DEFINE_END(vp__PolicyFcWrapHandler)
};

}  // namespace vp

#endif