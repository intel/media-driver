/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     vp_user_feature_control.h
//! \brief    vp user feature control.
//!
#ifndef __VP_USER_FEATURE_CONTROL_H__
#define __VP_USER_FEATURE_CONTROL_H__

#include "mos_os.h"
#include "vp_pipeline_common.h"

namespace vp
{

class VpUserFeatureControl
{
public:
    VpUserFeatureControl(MOS_INTERFACE &osInterface, void *owner = nullptr);
    virtual ~VpUserFeatureControl();

    struct CONTROL_VALUES
    {
        // Control values
        bool disableVeboxOutput             = false; // If true (VPHAL_COMP_BYPASS_DISABLED), output from vebox directly is not allowed, otherwise, output from vebox is allowed.
        bool disableSfc                     = false;
        bool computeContextEnabled          = true;
        bool eufusionBypassWaEnabled        = true;
    };

    virtual MOS_STATUS Update(PVP_PIPELINE_PARAMS params);

    bool IsVeboxOutputDisabled()
    {
        return m_ctrlVal.disableVeboxOutput;
    }

    bool IsSfcDisabled()
    {
        return m_ctrlVal.disableSfc;
    }

    bool IsComputeContextEnabled()
    {
        return m_ctrlVal.computeContextEnabled;
    }

    bool IsEufusionBypassWaEnabled()
    {
        return m_ctrlVal.eufusionBypassWaEnabled;
    }

    const void *m_owner = nullptr; // The object who create current instance.

protected:
    PMOS_INTERFACE m_osInterface;

    CONTROL_VALUES m_ctrlValDefault = {};
    CONTROL_VALUES m_ctrlVal        = {};

MEDIA_CLASS_DEFINE_END(VpUserFeatureControl)
};


}
#endif // !__VP_USER_FEATURE_CONTROL_H__
