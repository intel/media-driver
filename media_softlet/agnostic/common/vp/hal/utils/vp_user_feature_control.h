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
#include "vp_platform_interface.h"

namespace vp
{

class VpUserFeatureControl
{
public:
    VpUserFeatureControl(MOS_INTERFACE &osInterface, VpPlatformInterface *vpPlatformInterface, void *owner = nullptr);
    virtual ~VpUserFeatureControl();

    struct CONTROL_VALUES
    {
        // Control values
        bool disableVeboxOutput             = false; // If true (VPHAL_COMP_BYPASS_DISABLED), output from vebox directly is not allowed, otherwise, output from vebox is allowed.
        bool disableSfc                     = false;
        bool computeContextEnabled          = true;
        bool eufusionBypassWaEnabled        = false;
        bool bypassVeboxDnStateUpdate       = false;
        bool cscCosffPatchModeDisabled      = false;

#if (_DEBUG || _RELEASE_INTERNAL)
        bool forceDecompressedOutput           = false;
        uint32_t enableSFCNv12P010LinearOutput = 0;
        uint32_t enableSFCRGBPRGB24Output      = 0;
        bool     cpOutputSurfaceInitEnabled    = false;
#endif
    };

#if (_DEBUG || _RELEASE_INTERNAL)
    bool IsForceDecompressedOutput()
    {
        return m_ctrlVal.forceDecompressedOutput;
    }

    uint32_t IsEnableSFCNv12P010LinearOutput()
    {
        return m_ctrlVal.enableSFCNv12P010LinearOutput;
    }

    uint32_t IsEnableSFCRGBPRGB24Output()
    {
        return m_ctrlVal.enableSFCRGBPRGB24Output;
    }

    bool IsCpOutputSurfaceInitEnabled()
    {
        return m_ctrlVal.cpOutputSurfaceInitEnabled;
    }
#endif

    virtual MOS_STATUS CreateUserSettingForDebug();

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

    bool IsBypassVeboxDnStateUpdate()
    {
        return m_ctrlVal.bypassVeboxDnStateUpdate;
    }

    bool IsCscCosffPatchModeDisabled()
    {
        return m_ctrlVal.cscCosffPatchModeDisabled;
    }

    const void *m_owner = nullptr; // The object who create current instance.

protected:
    PMOS_INTERFACE m_osInterface = nullptr;
    VpPlatformInterface *m_vpPlatformInterface = nullptr;

    CONTROL_VALUES m_ctrlValDefault = {};
    CONTROL_VALUES m_ctrlVal        = {};
    MediaUserSettingSharedPtr m_userSettingPtr = nullptr;  //!< usersettingInstance

    MEDIA_CLASS_DEFINE_END(vp__VpUserFeatureControl)
};


}
#endif // !__VP_USER_FEATURE_CONTROL_H__
