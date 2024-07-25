/*
* Copyright (c) 2020-2024, Intel Corporation
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
#include "vp_hal_ddi_utils.h"
#include "mhw_sfc_g12_X.h"
#include "mos_defs.h"

using namespace vp;

SfcRenderM12::SfcRenderM12(
    VP_MHWINTERFACE &vpMhwinterface,
    PVpAllocator &allocator,
    bool disbaleSfcDithering) :
    SfcRenderBaseLegacy(vpMhwinterface, allocator, disbaleSfcDithering)
{
}

SfcRenderM12::~SfcRenderM12()
{
}

MOS_STATUS SfcRenderM12::SetupSfcState(
    PVP_SURFACE                     targetSurface)
{
    VP_FUNC_CALL();

    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;
    PMHW_SFC_STATE_PARAMS_G12 sfcStateParamsM12 = nullptr;

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBaseLegacy::SetupSfcState(targetSurface));

    //Set SFD Line Buffer
    VP_RENDER_CHK_NULL_RETURN(m_renderDataLegacy.sfcStateParams);
    sfcStateParamsM12 = static_cast<PMHW_SFC_STATE_PARAMS_G12>(m_renderDataLegacy.sfcStateParams);
    VP_RENDER_CHK_NULL_RETURN(sfcStateParamsM12);

    if (m_renderData.b1stPassOfSfc2PassScaling)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(sfcStateParamsM12->resAvsLineBuffer, m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass[m_scalabilityParams.curPipe]));
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(sfcStateParamsM12->resIefLineBuffer, m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass[m_scalabilityParams.curPipe]));
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(sfcStateParamsM12->resSfdLineBuffer, m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass[m_scalabilityParams.curPipe]));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(sfcStateParamsM12->resAvsLineBuffer, m_AVSLineBufferSurfaceArray[m_scalabilityParams.curPipe]));
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(sfcStateParamsM12->resIefLineBuffer, m_IEFLineBufferSurfaceArray[m_scalabilityParams.curPipe]));
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(sfcStateParamsM12->resSfdLineBuffer, m_SFDLineBufferSurfaceArray[m_scalabilityParams.curPipe]));
    }

    VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(sfcStateParamsM12->resAvsLineTileBuffer, m_AVSLineTileBufferSurface));
    VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(sfcStateParamsM12->resIefLineTileBuffer, m_IEFLineTileBufferSurface));
    VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(sfcStateParamsM12->resSfdLineTileBuffer, m_SFDLineTileBufferSurface));

    sfcStateParamsM12->histogramSurface = &m_histogramSurf;

    return eStatus;
}

MOS_STATUS SfcRenderM12::InitSfcStateParams()
{
    VP_FUNC_CALL();

    if (nullptr == m_sfcStateParamsLegacy)
    {
        m_sfcStateParamsLegacy = (MHW_SFC_STATE_PARAMS_G12*)MOS_AllocAndZeroMemory(sizeof(MHW_SFC_STATE_PARAMS_G12));
    }
    else
    {
        MOS_ZeroMemory(m_sfcStateParamsLegacy, sizeof(MHW_SFC_STATE_PARAMS_G12));
    }

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcStateParamsLegacy);

    m_renderDataLegacy.sfcStateParams = m_sfcStateParamsLegacy;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderM12::SetCodecPipeMode(CODECHAL_STANDARD codecStandard)
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    if (CODECHAL_HEVC != m_videoConfig.codecStandard &&
        CODECHAL_VP9 != m_videoConfig.codecStandard)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (CODECHAL_HEVC == m_videoConfig.codecStandard)
    {
        sfcStateParams->dwVDVEInputOrderingMode = (16 == m_videoConfig.hevc.lcuSize) ? MhwSfcInterfaceG12::LCU_16_16_HEVC :
            (32 == m_videoConfig.hevc.lcuSize) ? MhwSfcInterfaceG12::LCU_32_32_HEVC : MhwSfcInterfaceG12::LCU_64_64_HEVC;
    }
    else if (CODECHAL_VP9 == m_videoConfig.codecStandard)
    {
        VPHAL_COLORPACK colorPack = VpHalDDIUtils::GetSurfaceColorPack(m_renderDataLegacy.SfcInputFormat);

        if ((VPHAL_COLORPACK_420 == colorPack)
            || (VPHAL_COLORPACK_444 == colorPack))
        {
            sfcStateParams->dwVDVEInputOrderingMode = MhwSfcInterfaceG12::LCU_64_64_VP9;
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
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_miInterface);

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBaseLegacy::AddSfcLock(
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

MOS_STATUS SfcRenderM12::SetupScalabilityParams()
{

    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_renderDataLegacy.sfcStateParams);
    PMHW_SFC_STATE_PARAMS_G12 sfcStateParams = static_cast<PMHW_SFC_STATE_PARAMS_G12>(m_renderDataLegacy.sfcStateParams);

    if (MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP != m_pipeMode &&
        MhwSfcInterface::SFC_PIPE_MODE_VEBOX!= m_pipeMode)
    {
        VP_RENDER_NORMALMESSAGE("No scalability params need be applied for pipeMode(%d)", m_pipeMode);
        return MOS_STATUS_SUCCESS;
    }

    if (1 == m_scalabilityParams.numPipe)
    {
        VP_RENDER_NORMALMESSAGE("Scalability is disabled.");
        return MOS_STATUS_SUCCESS;
    }

    // Check whether engine mode being valid.
    uint32_t engineMode = (0 == m_scalabilityParams.curPipe) ? 1 :
                        (m_scalabilityParams.numPipe - 1 == m_scalabilityParams.curPipe) ? 2 : 3;
    if (engineMode != m_scalabilityParams.engineMode)
    {
        VP_RENDER_ASSERTMESSAGE("engineMode (%d) may not be expected according to curPipe(%d) and numPipe(%d).",
            m_scalabilityParams.engineMode, m_scalabilityParams.curPipe, m_scalabilityParams.numPipe);
    }

    sfcStateParams->engineMode = m_scalabilityParams.engineMode;

    if (MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP == m_pipeMode)
    {
        VPHAL_COLORPACK colorPack = VpHalDDIUtils::GetSurfaceColorPack(m_renderDataLegacy.SfcInputFormat);

        if ((VPHAL_COLORPACK_420 == colorPack || VPHAL_COLORPACK_422 == colorPack) &&
            (!MOS_IS_ALIGNED(m_scalabilityParams.srcStartX, 2) || MOS_IS_ALIGNED(m_scalabilityParams.srcEndX, 2)))
        {
            VP_PUBLIC_ASSERTMESSAGE("srcStartX(%d) is not even or srcEndX(%d) is not odd with input format(%d).",
                 m_scalabilityParams.srcStartX, m_scalabilityParams.srcEndX, m_renderDataLegacy.SfcInputFormat);
        }
        sfcStateParams->tileType     = m_scalabilityParams.tileType;
        sfcStateParams->srcStartX    = m_scalabilityParams.srcStartX;
        sfcStateParams->srcEndX      = m_scalabilityParams.srcEndX;
        sfcStateParams->dstStartX    = m_scalabilityParams.dstStartX;
        sfcStateParams->dstEndX      = m_scalabilityParams.dstEndX;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Set sfc pipe selected with vebox
//! \details  Set sfc pipe selected with vebox
//! \param    [in] dwSfcPipe
//!           Sfc pipe selected with vebox
//! \param    [in] dwSfcNum
//!           Sfc pipe num in total
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
MOS_STATUS SfcRenderM12::SetSfcPipe(
    uint32_t dwSfcPipe,
    uint32_t dwSfcNum)
{
    VP_FUNC_CALL();

    MOS_STATUS         eStatus = MOS_STATUS_SUCCESS;

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcInterface);
    PMHW_SFC_INTERFACE pSfcInterface = static_cast<PMHW_SFC_INTERFACE>(m_sfcInterface);

    if (dwSfcPipe >= dwSfcNum)
    {
        VP_PUBLIC_ASSERTMESSAGE("Scalability sfc pipe set by vebox, dwSfcPipe %d, dwSfcNum %d", dwSfcPipe, dwSfcNum);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_scalabilityParams.curPipe    = dwSfcPipe;
    m_scalabilityParams.numPipe    = dwSfcNum;
    m_scalabilityParams.engineMode = (0 == m_scalabilityParams.curPipe) ? 1 : (m_scalabilityParams.numPipe - 1 == m_scalabilityParams.curPipe) ? 2 : 3;

    pSfcInterface = m_sfcInterface;

    pSfcInterface->SetSfcIndex(dwSfcPipe, dwSfcNum);

    return eStatus;
}

bool SfcRenderM12::IsOutputChannelSwapNeeded(MOS_FORMAT outputFormat)
{
    VP_FUNC_CALL();

    // ARGB8,ABGR10, output format need to enable swap
    // Only be used with RGB output formats and CSC conversion is turned on.
    if (outputFormat == Format_X8R8G8B8 ||
        outputFormat == Format_A8R8G8B8 ||
        outputFormat == Format_R10G10B10A2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SfcRenderM12::IsCscNeeded(SFC_CSC_PARAMS &cscParams)
{
    VP_FUNC_CALL();

    return cscParams.bCSCEnabled                        ||
        IsInputChannelSwapNeeded(cscParams.inputFormat) ||
        IsOutputChannelSwapNeeded(cscParams.outputFormat);
}
