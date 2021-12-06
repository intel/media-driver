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
//! \file     vp_user_feature_control.cpp
//! \brief    vp user feature control.
//! \details  vp user feature control.
//!
#include "vp_user_feature_control.h"
#include "vp_utils.h"

using namespace vp;

VpUserFeatureControl::VpUserFeatureControl(MOS_INTERFACE &osInterface, void *owner) : m_owner(owner), m_osInterface(&osInterface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_USER_FEATURE_VALUE_DATA userFeatureData = {};
    auto skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);

    // Read user feature key to get the Composition Bypass mode
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    // Vebox Comp Bypass is on by default
    userFeatureData.u32Data = VPHAL_COMP_BYPASS_ENABLED;

    status = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_BYPASS_COMPOSITION_ID,
        &userFeatureData,
        m_osInterface->pOsContext);

    if (MOS_SUCCEEDED(status))
    {
        m_ctrlValDefault.disableVeboxOutput = VPHAL_COMP_BYPASS_DISABLED == userFeatureData.u32Data;
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
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        status = MOS_UserFeature_ReadValue_ID(
            nullptr,
            __VPHAL_VEBOX_DISABLE_SFC_ID,
            &userFeatureData,
            m_osInterface->pOsContext);

        if (MOS_SUCCEEDED(status))
        {
            m_ctrlValDefault.disableSfc = userFeatureData.bData;
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
    }
    VP_PUBLIC_NORMALMESSAGE("disableSfc %d", m_ctrlValDefault.disableSfc);

    // bComputeContextEnabled is true only if Gen12+. 
    // Gen12+, compute context(MOS_GPU_NODE_COMPUTE, MOS_GPU_CONTEXT_COMPUTE) can be used for render engine.
    // Before Gen12, we only use MOS_GPU_NODE_3D and MOS_GPU_CONTEXT_RENDER.
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrCCSNode))
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_USER_FEATURE_VALUE_DATA UserFeatureData = {};
        status = MOS_UserFeature_ReadValue_ID(
            nullptr,
            __VPHAL_ENABLE_COMPUTE_CONTEXT_ID,
            &UserFeatureData,
            m_osInterface->pOsContext);

        if (MOS_SUCCEEDED(status))
        {
            m_ctrlValDefault.computeContextEnabled = UserFeatureData.bData ? true : false;
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

    MT_LOG3(MT_VP_USERFEATURE_CTRL, MT_NORMAL, MT_VP_UF_CTRL_DISABLE_VEOUT, m_ctrlValDefault.disableVeboxOutput,
        MT_VP_UF_CTRL_DISABLE_SFC, m_ctrlValDefault.disableSfc, MT_VP_UF_CTRL_CCS, m_ctrlValDefault.computeContextEnabled);

    m_ctrlVal = m_ctrlValDefault;
}

VpUserFeatureControl::~VpUserFeatureControl()
{
}

MOS_STATUS VpUserFeatureControl::Update(PVP_PIPELINE_PARAMS params)
{
    VP_PUBLIC_CHK_NULL_RETURN(params);

    m_ctrlVal = m_ctrlValDefault;

    return MOS_STATUS_SUCCESS;
}
