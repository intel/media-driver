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
        bool disableDn                      = false;
        bool cscCosffPatchModeDisabled      = false;
        bool ForceEnableVeboxOutputSurf     = false;
        bool veboxTypeH                     = false;
        bool is3DLutKernelOnly              = false;

#if (_DEBUG || _RELEASE_INTERNAL)
        bool forceDecompressedOutput        = false;
        uint32_t force3DLutInterpolation    = 0;
        uint32_t enabledSFCNv12P010LinearOutput = 0;
        uint32_t enabledSFCRGBPRGB24Output  = 0;
        bool     enableIFNCC                    = false;
#endif
        VP_CTRL enableOcl3DLut              = VP_CTRL_DEFAULT;
        VP_CTRL forceOclFC                  = VP_CTRL_DEFAULT;
        bool    bDisableOclFcFp             = false;

        bool disablePacketReuse             = false;
        bool enablePacketReuseTeamsAlways   = false;

        VPHAL_HDR_LUT_MODE globalLutMode      = VPHAL_HDR_LUT_MODE_NONE;  //!< Global LUT mode control for debugging purpose
        bool               gpuGenerate3DLUT   = false;                        //!< Flag for per frame GPU generation of 3DLUT
        bool               isExternal3DLutSupport  = true;
        bool               disableAutoMode    = false;
        bool               clearVideoViewMode = false;
        uint32_t           splitFramePortions = 1;
        bool               decompForInterlacedSurfWaEnabled = false;
        bool               enableSFCLinearOutputByTileConvert = false;
    };

    uint32_t Is3DLutKernelOnly()
    {
        return m_ctrlVal.is3DLutKernelOnly;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    bool IsForceDecompressedOutput()
    {
        return m_ctrlVal.forceDecompressedOutput;
    }

    uint32_t Force3DLutInterpolation()
    {
        return m_ctrlVal.force3DLutInterpolation;
    }

    uint32_t EnabledSFCNv12P010LinearOutput()
    {
        return m_ctrlVal.enabledSFCNv12P010LinearOutput;
    }

    uint32_t EnabledSFCRGBPRGB24Output()
    {
        return m_ctrlVal.enabledSFCRGBPRGB24Output;
    }

    bool EnableIFNCC() 
    {
        return m_ctrlVal.enableIFNCC;
    }

    bool DisableOclFcFp()
    {
        return m_ctrlVal.bDisableOclFcFp;
    }
#endif

    bool IsSFCLinearOutputByTileConvertEnabled()
    {
        return m_ctrlVal.enableSFCLinearOutputByTileConvert;
    }

    virtual MOS_STATUS CreateUserSettingForDebug();

    virtual MOS_STATUS Update(PVP_PIPELINE_PARAMS params);

    bool EnableOclFC();

    bool EnableOcl3DLut();

    bool IsVeboxOutputSurfEnabled()
    {
        return m_ctrlVal.ForceEnableVeboxOutputSurf;
    }

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

    bool IsDisableDn()
    {
        return m_ctrlVal.disableDn;
    }

    bool IsCscCosffPatchModeDisabled()
    {
        return m_ctrlVal.cscCosffPatchModeDisabled;
    }

    bool IsPacketReuseDisabled()
    {
        return m_ctrlVal.disablePacketReuse;
    }

    bool IsPacketReuseEnabledTeamsAlways()
    {
        return m_ctrlVal.enablePacketReuseTeamsAlways;
    }

    uint32_t GetGlobalLutMode()
    {
        return m_ctrlVal.globalLutMode;
    }

    bool IsGpuGenerate3DLUT()
    {
        return m_ctrlVal.gpuGenerate3DLUT;
    }

    bool IsExternal3DLutSupport()
    {
        return m_ctrlVal.isExternal3DLutSupport;
    }

    bool IsDisableAutoMode()
    {
        return m_ctrlVal.disableAutoMode;
    }

    bool IsClearVideoViewMode()
    {
        return m_ctrlVal.clearVideoViewMode;
    }

    bool IsDecompForInterlacedSurfWaEnabled()
    {
        return m_ctrlVal.decompForInterlacedSurfWaEnabled;
    }

    bool IsVeboxTypeHMode()
    {
        return m_ctrlVal.veboxTypeH;
    }

    MOS_STATUS SetClearVideoViewMode(bool mode)
    {
        m_ctrlVal.clearVideoViewMode = mode;
        return MOS_STATUS_SUCCESS;
    }

    uint32_t GetSplitFramePortions()
    {
        return m_ctrlVal.splitFramePortions;
    }

    MOS_STATUS ForceRenderPath(bool status)
    {
        m_ctrlVal.disableSfc                = status;
        m_ctrlVal.disableVeboxOutput        = status;
        m_ctrlValDefault.disableSfc         = status;
        m_ctrlValDefault.disableVeboxOutput = status;

        return MOS_STATUS_SUCCESS;
    }

    virtual PMOS_OCA_LOG_USER_FEATURE_CONTROL_INFO GetOcaFeautreControlInfo();

    const void *m_owner = nullptr; // The object who create current instance.

protected:
    PMOS_INTERFACE m_osInterface = nullptr;
    VpPlatformInterface *m_vpPlatformInterface = nullptr;

    CONTROL_VALUES m_ctrlValDefault = {};
    CONTROL_VALUES m_ctrlVal        = {};
    MediaUserSettingSharedPtr m_userSettingPtr = nullptr;  //!< usersettingInstance
    PMOS_OCA_LOG_USER_FEATURE_CONTROL_INFO m_pOcaFeatureControlInfo = nullptr;

    MEDIA_CLASS_DEFINE_END(vp__VpUserFeatureControl)
};


}
#endif // !__VP_USER_FEATURE_CONTROL_H__
