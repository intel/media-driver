/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     policy.h
//! \brief    Defines the common interface for vp features manager
//! \details  The policy is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __POLICY_H__
#define __POLICY_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"
#include "vp_feature_caps.h"

#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_resource_manager.h"
#include <map>

namespace vp
{
#define ENGINE_MUST_MASK(supported)    (supported & 0x02)
#define ENGINE_SUPPORT_MASK(supported) (supported & 0x3)
#define ENGINE_MUST(supported)         (supported << 1)
#define FEATURE_TYPE_EXECUTE(feature, engine) FeatureType##feature##On##engine

class VpInterface;

class Policy
{
public:
    Policy(VpInterface &vpInterface);
    virtual ~Policy();
    MOS_STATUS CreateHwFilter(SwFilterPipe &subSwFilterPipe, HwFilter *&pFilter);
    MOS_STATUS Initialize();

protected:
    MOS_STATUS GetHwFilterParam(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params);
    MOS_STATUS GetExecuteCaps(SwFilterPipe &subSwFilterPipe, VP_EXECUTE_CAPS &caps);

    MOS_STATUS RegisterFeatures();
    MOS_STATUS ReleaseHwFilterParam(HW_FILTER_PARAMS &params);
    MOS_STATUS GetExecuteCaps(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params);
    MOS_STATUS BuildExecutionEngines(SwFilterSubPipe& SwFilterPipe);

    MOS_STATUS GetCSCExecutionCaps(SwFilter* feature);
    MOS_STATUS GetScalingExecutionCaps(SwFilter* feature);
    MOS_STATUS GetRotationExecutionCaps(SwFilter* feature);
    MOS_STATUS GetExecutionCaps(SwFilter* feature);

    MOS_STATUS BuildFilters(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params);
    MOS_STATUS BuildExecuteFilter(SwFilterPipe& subSwFilterPipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params);
    MOS_STATUS SetupExecuteFilter(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params);
    MOS_STATUS UpdateExeCaps(SwFilter* feature, VP_EXECUTE_CAPS& caps, EngineType Type);

    std::map<FeatureType, PolicyFeatureHandler*> m_VeboxSfcFeatureHandlers;
    std::map<FeatureType, PolicyFeatureHandler*> m_RenderFeatureHandlers;
    std::vector<FeatureType> m_featurePool;

    VpInterface         &m_vpInterface;
    VP_SFC_ENTRY_REC    m_sfcHwEntry[Format_Count] = {};
};

}
#endif // !__POLICY_H__