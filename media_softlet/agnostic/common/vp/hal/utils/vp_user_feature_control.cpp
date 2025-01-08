/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     vp_user_feature_control.cpp
//! \brief    vp user feature control.
//! \details  vp user feature control.
//!
#include "vp_user_feature_control.h"
#include "vp_utils.h"

using namespace vp;

VpUserFeatureControl::VpUserFeatureControl(MOS_INTERFACE &osInterface, VpPlatformInterface *vpPlatformInterface, void *owner) :
    m_owner(owner), m_osInterface(&osInterface), m_vpPlatformInterface(vpPlatformInterface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    uint32_t compBypassMode = VPHAL_COMP_BYPASS_ENABLED;    // Vebox Comp Bypass is on by default
    auto skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);

    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    // Read user feature key to get the Composition Bypass mode
    if (skuTable && (!MEDIA_IS_SKU(skuTable, FtrVERing)))
    {
        m_ctrlValDefault.disableVeboxOutput = true;
        m_ctrlValDefault.disableSfc         = true;

        VP_PUBLIC_NORMALMESSAGE("No VeRing, disableVeboxOutput %d, disableSfc %d", m_ctrlValDefault.disableVeboxOutput, m_ctrlValDefault.disableSfc);
    }
    else
    {
        status = ReadUserSetting(
            m_userSettingPtr,
            compBypassMode,
            __VPHAL_BYPASS_COMPOSITION,
            MediaUserSetting::Group::Sequence,
            compBypassMode,
            true);

        if (MOS_SUCCEEDED(status))
        {
            m_ctrlValDefault.disableVeboxOutput = VPHAL_COMP_BYPASS_DISABLED == compBypassMode;
        }
        else
        {
            // Default value
            m_ctrlValDefault.disableVeboxOutput = false;
        }

        VP_PUBLIC_NORMALMESSAGE("disableVeboxOutput %d", m_ctrlValDefault.disableVeboxOutput);

        if (skuTable && MEDIA_IS_SKU(skuTable, FtrSFCPipe))
        {
            // Read user feature key to Disable SFC
            bool disableSFC = false;
            status          = ReadUserSetting(
                m_userSettingPtr,
                disableSFC,
                __VPHAL_VEBOX_DISABLE_SFC,
                MediaUserSetting::Group::Sequence);

            if (MOS_SUCCEEDED(status))
            {
                m_ctrlValDefault.disableSfc = disableSFC;
            }
            else
            {
                // Default value
                m_ctrlValDefault.disableSfc = false;
            }
        }
        else
        {
            m_ctrlValDefault.disableSfc = true;
            VP_PUBLIC_NORMALMESSAGE("No FtrSFCPipe, disableSfc %d", m_ctrlValDefault.disableSfc);
        }
        VP_PUBLIC_NORMALMESSAGE("disableSfc %d", m_ctrlValDefault.disableSfc);
    }

    // If disable dn
    bool disableDn = false;
    status = ReadUserSetting(
        m_userSettingPtr,
        disableDn,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_DN,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.disableDn = disableDn;
    }
    else
    {
        // Default value
        m_ctrlValDefault.disableDn = false;
    }
    VP_PUBLIC_NORMALMESSAGE("disableDn %d", m_ctrlValDefault.disableDn);

   // Force enable vebox ouptut surface.
    bool forceEnableVeboxOutputSurf = false;
    status = ReadUserSetting(
        m_userSettingPtr,
        forceEnableVeboxOutputSurf,
        __MEDIA_USER_FEATURE_VALUE_FORCE_ENABLE_VEBOX_OUTPUT_SURF,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.ForceEnableVeboxOutputSurf = forceEnableVeboxOutputSurf;
    }
    else
    {
        // Default value
        m_ctrlValDefault.ForceEnableVeboxOutputSurf = false;
    }
    VP_PUBLIC_NORMALMESSAGE("ForceEnableVeboxOutputSurf %d", m_ctrlValDefault.ForceEnableVeboxOutputSurf);

    // __MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE
    bool cscCoeffPatchModeDisabled = false;
    status = ReadUserSetting(
        m_userSettingPtr,
        cscCoeffPatchModeDisabled,
        __MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.cscCosffPatchModeDisabled = cscCoeffPatchModeDisabled;
    }
    else
    {
        // Default value
        m_ctrlValDefault.cscCosffPatchModeDisabled = false;
    }
    VP_PUBLIC_NORMALMESSAGE("cscCosffPatchModeDisabled %d", m_ctrlValDefault.cscCosffPatchModeDisabled);

    bool disablePacketReuse = false;
    status = ReadUserSetting(
        m_userSettingPtr,
        disablePacketReuse,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_PACKET_REUSE,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.disablePacketReuse = disablePacketReuse;
    }
    else
    {
        // Default value
        m_ctrlValDefault.disablePacketReuse = false;
    }
    VP_PUBLIC_NORMALMESSAGE("disablePacketReuse %d", m_ctrlValDefault.disablePacketReuse);

    bool enablePacketReuseTeamsAlways = false;
    status = ReadUserSetting(
        m_userSettingPtr,
        enablePacketReuseTeamsAlways,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_PACKET_REUSE_TEAMS_ALWAYS,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.enablePacketReuseTeamsAlways = enablePacketReuseTeamsAlways;
    }
    else
    {
        // Default value
        m_ctrlValDefault.enablePacketReuseTeamsAlways = false;
    }
    VP_PUBLIC_NORMALMESSAGE("enablePacketReuseTeamsAlways %d", m_ctrlValDefault.enablePacketReuseTeamsAlways);

    // bComputeContextEnabled is true only if Gen12+. 
    // Gen12+, compute context(MOS_GPU_NODE_COMPUTE, MOS_GPU_CONTEXT_COMPUTE) can be used for render engine.
    // Before Gen12, we only use MOS_GPU_NODE_3D and MOS_GPU_CONTEXT_RENDER.
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrCCSNode))
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        bool computeContextEnabled = false;
        status = ReadUserSettingForDebug(
            m_userSettingPtr,
            computeContextEnabled,
            __VPHAL_ENABLE_COMPUTE_CONTEXT,
            MediaUserSetting::Group::Sequence);

        if (MOS_SUCCEEDED(status))
        {
            m_ctrlValDefault.computeContextEnabled = computeContextEnabled ? true : false;
        }
        else
#endif
        {
            // Default value
            m_ctrlValDefault.computeContextEnabled = true;
        }
    }
    else
    {
        m_ctrlValDefault.computeContextEnabled = false;
    }
    VP_PUBLIC_NORMALMESSAGE("computeContextEnabled %d", m_ctrlValDefault.computeContextEnabled);

    // Read userSettingForDebug
    CreateUserSettingForDebug();

    if (m_vpPlatformInterface)
    {
        m_ctrlValDefault.eufusionBypassWaEnabled = m_vpPlatformInterface->IsEufusionBypassWaEnabled();
        m_ctrlValDefault.decompForInterlacedSurfWaEnabled = m_vpPlatformInterface->IsDecompForInterlacedSurfWaEnabled();
    }
    else
    {
        // Should never come to here.
        VP_PUBLIC_ASSERTMESSAGE("m_vpPlatformInterface == nullptr");
    }
    VP_PUBLIC_NORMALMESSAGE("eufusionBypassWaEnabled %d", m_ctrlValDefault.eufusionBypassWaEnabled);
    VP_PUBLIC_NORMALMESSAGE("decompForInterlacedSurfWaEnabled %d", m_ctrlValDefault.decompForInterlacedSurfWaEnabled);

    MT_LOG3(MT_VP_USERFEATURE_CTRL, MT_NORMAL, MT_VP_UF_CTRL_DISABLE_VEOUT, m_ctrlValDefault.disableVeboxOutput,
        MT_VP_UF_CTRL_DISABLE_SFC, m_ctrlValDefault.disableSfc, MT_VP_UF_CTRL_CCS, m_ctrlValDefault.computeContextEnabled);

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t globalLutMode = VPHAL_HDR_LUT_MODE_NONE;
    status                 = ReadUserSetting(
        m_userSettingPtr,
        globalLutMode,
        __VPHAL_HDR_LUT_MODE,
        MediaUserSetting::Group::Sequence,
        VPHAL_HDR_LUT_MODE_NONE,
        true);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.globalLutMode = (VPHAL_HDR_LUT_MODE)globalLutMode;
    }

    bool gpuGenerate3DLUT = false;
    status                = ReadUserSetting(
        m_userSettingPtr,
        gpuGenerate3DLUT,
        __VPHAL_HDR_GPU_GENERTATE_3DLUT,
        MediaUserSetting::Group::Sequence,
        false,
        true);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.gpuGenerate3DLUT = gpuGenerate3DLUT;
    }

    bool disableAutoMode = false;
    status               = ReadUserSetting(
        m_userSettingPtr,
        disableAutoMode,
        __VPHAL_HDR_DISABLE_AUTO_MODE,
        MediaUserSetting::Group::Sequence,
        false,
        true);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.disableAutoMode = disableAutoMode;
    }
#endif

    uint32_t splitFramePortions = 1;
    status                      = ReadUserSetting(
        m_userSettingPtr,
        splitFramePortions,
        __VPHAL_HDR_SPLIT_FRAME_PORTIONS,
        MediaUserSetting::Group::Sequence,
        splitFramePortions,
        true);
    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.splitFramePortions = splitFramePortions;
    }

    //check vebox type 
    if (skuTable && (MEDIA_IS_SKU(skuTable, FtrVeboxTypeH)))
    {
        m_ctrlValDefault.veboxTypeH = true;
    }
    VP_PUBLIC_NORMALMESSAGE("veboxTypeH %d", m_ctrlValDefault.veboxTypeH);

    m_ctrlVal = m_ctrlValDefault;
}

VpUserFeatureControl::~VpUserFeatureControl()
{
    if (m_pOcaFeatureControlInfo)
    {
        MOS_FreeMemAndSetNull(m_pOcaFeatureControlInfo);
    }
}

MOS_STATUS VpUserFeatureControl::CreateUserSettingForDebug()
{
    MOS_STATUS eRegKeyReadStatus = MOS_STATUS_SUCCESS;
#if ((_DEBUG || _RELEASE_INTERNAL) && !EMUL)
    bool forceDecompressedOutput = false;
    eRegKeyReadStatus = ReadUserSettingForDebug(
        m_userSettingPtr,
        forceDecompressedOutput,
        __VPHAL_RNDR_FORCE_VP_DECOMPRESSED_OUTPUT,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.forceDecompressedOutput = forceDecompressedOutput;
    }
    else
    {
        // Default value
        m_ctrlValDefault.forceDecompressedOutput = false;
    }
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t   force3DLutInterpolation = 0;
    eRegKeyReadStatus                  = ReadUserSettingForDebug(
        m_userSettingPtr,
        force3DLutInterpolation,
        __VPHAL_FORCE_3DLUT_INTERPOLATION,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.force3DLutInterpolation = force3DLutInterpolation;
    }
    else
    {
        // Default value
        m_ctrlValDefault.force3DLutInterpolation = 0;
    }

    uint32_t is3DLutKernelOnly = 0;
    eRegKeyReadStatus                = ReadUserSettingForDebug(
        m_userSettingPtr,
        is3DLutKernelOnly,
        __VPHAL_FORCE_VP_3DLUT_KERNEL_ONLY,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.is3DLutKernelOnly = is3DLutKernelOnly;
    }
    else
    {
        // Default value
        m_ctrlValDefault.is3DLutKernelOnly = 0;
    }

    //SFC NV12/P010 Linear Output.
    uint32_t enabledSFCNv12P010LinearOutput = 0;
    eRegKeyReadStatus = ReadUserSettingForDebug(
        m_userSettingPtr,
        enabledSFCNv12P010LinearOutput,
        __VPHAL_ENABLE_SFC_NV12_P010_LINEAR_OUTPUT,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.enabledSFCNv12P010LinearOutput = enabledSFCNv12P010LinearOutput;
    }
    else
    {
        // Default value
        m_ctrlValDefault.enabledSFCNv12P010LinearOutput = 0;
    }

    //SFC RGBP Linear/Tile RGB24 Linear Output.
    uint32_t enabledSFCRGBPRGB24Output = 0;
    eRegKeyReadStatus =ReadUserSettingForDebug(
        m_userSettingPtr,
        enabledSFCRGBPRGB24Output,
        __VPHAL_ENABLE_SFC_RGBP_RGB24_OUTPUT,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.enabledSFCRGBPRGB24Output = enabledSFCRGBPRGB24Output;
    }
    else
    {
        // Default value
        m_ctrlValDefault.enabledSFCRGBPRGB24Output = 0;
    }

    //Enable IFNCC report
    bool enableIFNCC  = false;
    eRegKeyReadStatus = ReadUserSettingForDebug(
        m_userSettingPtr,
        enableIFNCC,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_IFNCC,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.enableIFNCC = enableIFNCC;
        VP_PUBLIC_NORMALMESSAGE("Report IFNCC Reg Key Enabled. Will Report IFNCC Reg key at the end of VP pipeline.");
    }
    else
    {
        // Default value
        m_ctrlValDefault.enableIFNCC = false;
    }

    bool bEnableOCL3DLut = false;
    eRegKeyReadStatus   = ReadUserSettingForDebug(
        m_userSettingPtr,
        bEnableOCL3DLut,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VP_OCL_3DLUT,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.enableOcl3DLut = bEnableOCL3DLut ? VP_CTRL_ENABLE : VP_CTRL_DISABLE;
    }
    else
    {
        // Default value
        m_ctrlValDefault.enableOcl3DLut = VP_CTRL_DEFAULT;
    }

    bool bForceOclFC   = false;
    eRegKeyReadStatus = ReadUserSettingForDebug(
        m_userSettingPtr,
        bForceOclFC,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VP_OCL_FC,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.forceOclFC = bForceOclFC ? VP_CTRL_ENABLE : VP_CTRL_DISABLE;
    }
    else
    {
        // Default value
        m_ctrlValDefault.forceOclFC = VP_CTRL_DEFAULT;
    }

    bool bDisableOclFcFp = false;
    eRegKeyReadStatus   = ReadUserSettingForDebug(
        m_userSettingPtr,
        bDisableOclFcFp,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_VP_OCL_FC_FP,
        MediaUserSetting::Group::Sequence);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.bDisableOclFcFp = bDisableOclFcFp;
    }
    else
    {
        // Default value
        m_ctrlValDefault.bDisableOclFcFp = false;
    }
#endif
#if (_DEBUG || _RELEASE_INTERNAL)
    bool enableSFCLinearOutputByTileConvert = 0;
    eRegKeyReadStatus   = ReadUserSettingForDebug(
        m_userSettingPtr,
        enableSFCLinearOutputByTileConvert,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VESFC_LINEAR_OUTPUT_BY_TILECONVERT,
        MediaUserSetting::Group::Device);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.enableSFCLinearOutputByTileConvert = enableSFCLinearOutputByTileConvert;
    }
    else
#endif
    {
        auto *waTable = m_osInterface->pfnGetWaTable(m_osInterface);
        // Default value
        m_ctrlValDefault.enableSFCLinearOutputByTileConvert = MEDIA_IS_WA(waTable, Wa_15016458807);
    }
    VP_PUBLIC_NORMALMESSAGE("enableSFCLinearOutputByTileConvert value is set as %d.", m_ctrlValDefault.enableSFCLinearOutputByTileConvert);

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t fallbackScalingToRender8K = 0;
    eRegKeyReadStatus                  = ReadUserSettingForDebug(
        m_userSettingPtr,
        fallbackScalingToRender8K,
        __MEDIA_USER_FEATURE_VALUE_FALLBACK_SCALING_TO_RENDER_8K,
        MediaUserSetting::Group::Sequence,
        true,
        true);
    if (MOS_SUCCEEDED(eRegKeyReadStatus))
    {
        m_ctrlValDefault.fallbackScalingToRender8K = fallbackScalingToRender8K;
    }
    else
#endif
    {
        // WA ID need be added before code merge.
        m_ctrlValDefault.fallbackScalingToRender8K = 1;
    }
    VP_PUBLIC_NORMALMESSAGE("fallbackScalingToRender8K %d", m_ctrlValDefault.fallbackScalingToRender8K);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpUserFeatureControl::Update(PVP_PIPELINE_PARAMS params)
{
    VP_PUBLIC_CHK_NULL_RETURN(params);

    m_ctrlVal = m_ctrlValDefault;

    if (params->bForceToRender)
    {
        m_ctrlVal.disableSfc         = true;
        m_ctrlVal.disableVeboxOutput = true;
        VP_PUBLIC_NORMALMESSAGE("Disable SFC and vebox output as task is forced to render.");
    }

    return MOS_STATUS_SUCCESS;
}

PMOS_OCA_LOG_USER_FEATURE_CONTROL_INFO VpUserFeatureControl::GetOcaFeautreControlInfo()
{
    if (nullptr == m_pOcaFeatureControlInfo)
    {
        m_pOcaFeatureControlInfo = (PMOS_OCA_LOG_USER_FEATURE_CONTROL_INFO)MOS_AllocAndZeroMemory(sizeof(MOS_OCA_LOG_USER_FEATURE_CONTROL_INFO));
    }
    return m_pOcaFeatureControlInfo;
}


bool VpUserFeatureControl::EnableOclFC()
{
    bool bEnableOclFC = false;
    if (m_ctrlVal.forceOclFC == VP_CTRL_ENABLE)
    {
        bEnableOclFC = true;
    }
    else if (m_ctrlVal.forceOclFC == VP_CTRL_DISABLE)
    {
        bEnableOclFC = false;
    }
    else // if (m_ctrlVal.forceOclFC == VP_CTRL_DEFAULT)
    {
        bEnableOclFC = (m_vpPlatformInterface && m_vpPlatformInterface->IsOclKernelEnabled());
    }
    VP_PUBLIC_NORMALMESSAGE("EnableOclFC set as %d", bEnableOclFC);
    return bEnableOclFC;
}

bool VpUserFeatureControl::EnableOcl3DLut()
{
    bool bEnableOcl3DLut = false;
    if (m_ctrlVal.enableOcl3DLut == VP_CTRL_ENABLE)
    {
        bEnableOcl3DLut = true;
    }
    else if (m_ctrlVal.enableOcl3DLut == VP_CTRL_DISABLE)
    {
        bEnableOcl3DLut = false;
    }
    else // if (m_ctrlVal.enableOcl3DLut == VP_CTRL_DEFAULT)
    {
        bEnableOcl3DLut = (m_vpPlatformInterface && m_vpPlatformInterface->IsOclKernelEnabled());
    }
    VP_PUBLIC_NORMALMESSAGE("EnableOcl3DLut set as %d", bEnableOcl3DLut);
    return bEnableOcl3DLut;
}
