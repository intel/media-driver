/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     vp_feature_manager.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __POLICY_H__
#define __POLICY_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"

#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include <map>

namespace vp
{

class Policy
{
public:
    Policy(PVP_MHWINTERFACE pHwInterface, bool bBypassSwFilterPipe);
    virtual ~Policy();
    MOS_STATUS CreateHwFilter(SwFilterPipe &subSwFilterPipe, HwFilterFactory &facotry, HwFilter *&pFilter);
    // Function for SwFilterPipe Disabled.
    MOS_STATUS CreateHwFilter(VP_PIPELINE_PARAMS &pipelineParams, HwFilterFactory &facotry, HwFilter *&pFilter);
    MOS_STATUS Initialize();

protected:
    MOS_STATUS GetHwFilterParam(SwFilterPipe &subSwFilterPipe, HW_FILTER_PARAMS &params);
    // Function for SwFilterPipe Disabled.
    MOS_STATUS GetHwFilterParam(VP_PIPELINE_PARAMS &pipelineParams, HW_FILTER_PARAMS &params);

    MOS_STATUS RegisterFeatures();
    MOS_STATUS ReturnHwFilterParam(HW_FILTER_PARAMS &params);
    MOS_STATUS GetExecuteCaps(VP_PIPELINE_PARAMS &pipelineParams, VP_EXECUTE_CAPS &caps);


    const bool          m_bBypassSwFilterPipe;
    PVP_MHWINTERFACE    m_pHwInterface;

    std::map<VpFeatureType, PolicyFeatureHandler*> m_VeboxSfcFeatureHandlers;
    std::map<VpFeatureType, PolicyFeatureHandler*> m_RenderFeatureHandlers;
};

}
#endif // !__POLICY_H__
