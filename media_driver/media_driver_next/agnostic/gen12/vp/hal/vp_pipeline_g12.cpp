/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     vp_pipeline_g12.cpp
//! \brief    Defines the interface for Gen12 vp pipeline
//!           this file is for the base interface which is shared by all features.
//!

#include "vp_pipeline_g12.h"
#include "media_user_settings_mgr_g12.h"
#include "vp_vebox_cmd_packet_g12.h"
#include "vp_packet_factory_g12.h"

namespace vp {

VpPipelineG12::VpPipelineG12( PMOS_INTERFACE osInterface, VphalFeatureReport *reporting ) :
    VpPipeline( osInterface, reporting )
{

}

MOS_STATUS VpPipelineG12::Init(void * settings)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(Initialize(settings));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipelineG12::Prepare(void * params)
{
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    VP_FUNC_CALL();

    // VP Execution Params Prepare
    eStatus = PrepareVpPipelineParams(params);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (eStatus == MOS_STATUS_UNIMPLEMENTED)
        {
            VP_PUBLIC_NORMALMESSAGE("Features are UNIMPLEMENTED on APG now \n");
            return eStatus;
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(eStatus);
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(PrepareVpExePipe());

    if (m_reporting)
    {
        m_reporting->OutputPipeMode = m_vpOutputPipe;

        if (m_mmc)
        {
            m_reporting->VPMMCInUse = m_mmc->IsMmcEnabled();
        }

        if (m_pvpParams->pSrc[0] && m_pvpParams->pSrc[0]->bCompressible)
        {
            m_reporting->PrimaryCompressible = true;
            m_reporting->PrimaryCompressMode = (uint8_t)(m_pvpParams->pSrc[0]->CompressionMode);
        }

        if (m_pvpParams->pTarget[0]->bCompressible)
        {
            m_reporting->RTCompressible = true;
            m_reporting->RTCompressMode = (uint8_t)(m_pvpParams->pTarget[0]->CompressionMode);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipelineG12::Execute()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(ExecuteVpPipeline())

    VP_PUBLIC_CHK_STATUS_RETURN(UserFeatureReport());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipelineG12::Destroy()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipelineG12::Initialize(void * settings)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(VpPipeline::Initialize(settings));

    VP_PUBLIC_CHK_STATUS_RETURN(GetSystemVeboxNumber());

    return MOS_STATUS_SUCCESS;
}

PacketFactory *VpPipelineG12::CreatePacketFactory()
{
    VP_FUNC_CALL();
    return MOS_New(PacketFactoryG12);
}

MOS_STATUS VpPipelineG12::PrepareVpExePipe()
{
    return VpPipeline::PrepareVpExePipe();
}

MOS_STATUS VpPipelineG12::GetSystemVeboxNumber()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(VpPipeline::GetSystemVeboxNumber());

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

    MOS_STATUS statusKey = MOS_STATUS_SUCCESS;
    statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE_ID_G12,
        &userFeatureData);

    bool disableScalability = false;
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        disableScalability = userFeatureData.i32Data ? true : false;
    }

    if (disableScalability)
    {
        m_numVebox = 1;
    }

    return eStatus;
}

}
