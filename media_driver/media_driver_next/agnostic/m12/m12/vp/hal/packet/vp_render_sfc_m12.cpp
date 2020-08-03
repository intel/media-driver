/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     vp_render_sfc_m12.cpp
//! \brief    SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#include "vp_render_sfc_m12.h"
#include "mhw_sfc_g12_X.h"
#include "mos_defs.h"

using namespace vp;

SfcRenderM12::SfcRenderM12(
    VP_MHWINTERFACE &vpMhwinterface,
    PVpAllocator &allocator):
    SfcRenderBase(vpMhwinterface, allocator)
{
}

SfcRenderM12::~SfcRenderM12()
{
}

MOS_STATUS SfcRenderM12::SetupSfcState(
    PVP_SURFACE                     targetSurface)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;
    PMHW_SFC_STATE_PARAMS_G12 sfcStateParamsM12 = nullptr;

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBase::SetupSfcState(targetSurface));

    //Set SFD Line Buffer
    VP_RENDER_CHK_NULL_RETURN(m_renderData.sfcStateParams);
    sfcStateParamsM12 = static_cast<PMHW_SFC_STATE_PARAMS_G12>(m_renderData.sfcStateParams);
    VP_RENDER_CHK_NULL_RETURN(sfcStateParamsM12);
    if (m_SFDLineBufferSurface && m_SFDLineBufferSurface->osSurface && !Mos_ResourceIsNull(&m_SFDLineBufferSurface->osSurface->OsResource))
    {
        sfcStateParamsM12->resSfdLineBuffer = &m_SFDLineBufferSurface->osSurface->OsResource;
    }
    else
    {
        sfcStateParamsM12->resSfdLineBuffer = nullptr;
    }

    return eStatus;
}

MOS_STATUS SfcRenderM12::InitSfcStateParams()
{
    if (nullptr == m_sfcStateParams)
    {
        m_sfcStateParams = (MHW_SFC_STATE_PARAMS_G12*)MOS_AllocAndZeroMemory(sizeof(MHW_SFC_STATE_PARAMS_G12));
    }
    else
    {
        MOS_ZeroMemory(m_sfcStateParams, sizeof(MHW_SFC_STATE_PARAMS_G12));
    }

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcStateParams);

    m_renderData.sfcStateParams = m_sfcStateParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderM12::SetCodecPipeMode(CODECHAL_STANDARD codecStandard)
{
    if (CODECHAL_HEVC == codecStandard ||
        CODECHAL_VP9 == codecStandard)
    {
        m_pipeMode = MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP;
    }
    else
    {
        return SfcRenderBase::SetCodecPipeMode(codecStandard);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderM12::SetSfcStateInputOrderingModeHcp(
    PMHW_SFC_STATE_PARAMS       sfcStateParams)
{
    if (CODECHAL_HEVC != m_videoConfig.codecStandard &&
        CODECHAL_VP9 != m_videoConfig.codecStandard)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (CODECHAL_HEVC == m_videoConfig.codecStandard)
    {
        sfcStateParams->dwVDVEInputOrderingMode = (16 == m_videoConfig.lcuSize) ? MhwSfcInterfaceG12::LCU_16_16_HEVC :
            (32 == m_videoConfig.lcuSize) ? MhwSfcInterfaceG12::LCU_32_32_HEVC : MhwSfcInterfaceG12::LCU_64_64_HEVC;
    }
    else if (CODECHAL_VP9 == m_videoConfig.codecStandard)
    {
        VPHAL_COLORPACK colorPack = VpHal_GetSurfaceColorPack(m_renderData.SfcInputFormat);
        if (VPHAL_COLORPACK_420 == colorPack)
        {
            sfcStateParams->dwVDVEInputOrderingMode = MhwSfcInterfaceG12::LCU_64_64_VP9;
        }
        else if (VPHAL_COLORPACK_444 == colorPack)
        {
            sfcStateParams->dwVDVEInputOrderingMode = MhwSfcInterfaceG12::LCU_64_64_VP9_ENC;
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderM12::AddSfcLock(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    PMHW_SFC_LOCK_PARAMS           pSfcLockParams)
{
    VP_RENDER_CHK_NULL_RETURN(m_miInterface);

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBase::AddSfcLock(
        pCmdBuffer,
        pSfcLockParams));

    //insert 2 dummy VD_CONTROL_STATE packets with data=0 after every HCP_SFC_LOCK
    if (MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP == m_pipeMode && MEDIA_IS_WA(m_waTable, Wa_14010222001))
    {
        MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
        MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
        for (int i = 0; i < 2; i++)
        {
            VP_RENDER_CHK_STATUS_RETURN(static_cast<MhwMiInterfaceG12 *>(m_miInterface)->AddMiVdControlStateCmd(pCmdBuffer, &vdCtrlParam));
        }
    }
    return MOS_STATUS_SUCCESS;
}