/*
* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     vp_render_sfc_xe3_lpm_base.cpp
//! \brief    SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#include "vp_render_sfc_xe3_lpm_base.h"
#include "mhw_sfc_itf.h"
#include "mos_defs.h"
#include "vp_hal_ddi_utils.h"

using namespace vp;

SfcRenderXe3_Lpm_Base::SfcRenderXe3_Lpm_Base(
    VP_MHWINTERFACE &vpMhwinterface,
    PVpAllocator &allocator,
    bool disbaleSfcDithering):
    SfcRenderBase(vpMhwinterface, allocator, disbaleSfcDithering)
{
}

SfcRenderXe3_Lpm_Base::~SfcRenderXe3_Lpm_Base()
{
    FreeResources();
}

MOS_STATUS SfcRenderXe3_Lpm_Base::SetupSfcState(
    PVP_SURFACE targetSurface)
{
    VP_FUNC_CALL();

    MOS_STATUS                eStatus           = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBase::SetupSfcState(targetSurface));

    //Set SFD Line Buffer
    VP_RENDER_CHK_NULL_RETURN(m_renderData.sfcStateParams);
    if (m_renderData.b1stPassOfSfc2PassScaling)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderData.sfcStateParams->resSfdLineBuffer, m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass[m_scalabilityParams.curPipe]));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderData.sfcStateParams->resSfdLineBuffer, m_SFDLineBufferSurfaceArray[m_scalabilityParams.curPipe]));
    }
    VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderData.sfcStateParams->resAvsLineTileBuffer, m_AVSLineTileBufferSurface));
    VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderData.sfcStateParams->resIefLineTileBuffer, m_IEFLineTileBufferSurface));
    VP_RENDER_CHK_STATUS_RETURN(SetLineBuffer(m_renderData.sfcStateParams->resSfdLineTileBuffer, m_SFDLineTileBufferSurface));

    if (mhw::sfc::SFC_PIPE_MODE_AVP == m_pipeMode &&
        CODECHAL_AV1 == m_videoConfig.codecStandard)
    {
        m_renderData.sfcStateParams->av1TileColumnNumber  = m_videoConfig.av1.tileCols;
        m_renderData.sfcStateParams->av1TileRowNumber     = m_videoConfig.av1.tileRows;
    }
    else
    {
        // Only for AV1 mode. Must be set to 0 for other modes.
        m_renderData.sfcStateParams->av1TileColumnNumber  = 0;
        m_renderData.sfcStateParams->av1TileRowNumber     = 0;
    }

    m_renderData.sfcStateParams->histogramSurface = &m_histogramSurf;

    return eStatus;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::SetCodecPipeMode(CODECHAL_STANDARD codecStandard)
{
    VP_FUNC_CALL();

    if (CODECHAL_AV1 == codecStandard)
    {
        m_pipeMode = mhw::sfc::SFC_PIPE_MODE_AVP;
    }
    else if (CODECHAL_HEVC == codecStandard ||
        CODECHAL_VP9 == codecStandard)
    {
        m_pipeMode = mhw::sfc::SFC_PIPE_MODE_HCP;
    }
    else
    {
        return SfcRenderBase::SetCodecPipeMode(codecStandard);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::SetSfcStateInputOrderingModeVdbox(
    mhw::sfc::SFC_STATE_PAR *sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    switch (m_videoConfig.codecStandard)
    {
    case CODECHAL_AV1:
        return SetSfcStateInputOrderingModeAvp(sfcStateParams);
    default:
        return SfcRenderBase::SetSfcStateInputOrderingModeVdbox(sfcStateParams);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::SetSfcStateInputOrderingModeAvp(
    mhw::sfc::SFC_STATE_PAR *sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    if (CODECHAL_AV1 != m_videoConfig.codecStandard)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (64 != m_videoConfig.av1.lcuSize && 128 != m_videoConfig.av1.lcuSize)
    {
        VP_PUBLIC_ASSERTMESSAGE("lcu size is %d, which is invalid!", m_videoConfig.av1.lcuSize);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    VPHAL_COLORPACK colorPack = VpHalDDIUtils::GetSurfaceColorPack(m_renderData.SfcInputFormat);

    if (VPHAL_COLORPACK_420 != colorPack)
    {
        VP_PUBLIC_ASSERTMESSAGE("The color pack of input surface is not 420 for AVP!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_videoConfig.av1.intraBC || m_videoConfig.av1.lossless && !m_videoConfig.av1.superResInuse)
    {
        sfcStateParams->dwVDVEInputOrderingMode = (64 == m_videoConfig.av1.lcuSize) ? mhw::sfc::LCU_64_64_NOSHIFT_AV1 : mhw::sfc::LCU_128_128_NOSHIFT_AV1;
    }
    else
    {
        sfcStateParams->dwVDVEInputOrderingMode = (64 == m_videoConfig.av1.lcuSize) ? mhw::sfc::LCU_64_64_SHIFT_AV1 : mhw::sfc::LCU_128_128_SHIFT_AV1;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::SetSfcStateInputOrderingModeHcp(
    mhw::sfc::SFC_STATE_PAR *sfcStateParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(sfcStateParams);
    if (CODECHAL_HEVC != m_videoConfig.codecStandard &&
        CODECHAL_VP9 != m_videoConfig.codecStandard)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (CODECHAL_HEVC == m_videoConfig.codecStandard)
    {
        sfcStateParams->dwVDVEInputOrderingMode = (16 == m_videoConfig.hevc.lcuSize) ? mhw::sfc::LCU_16_16_HEVC : (32 == m_videoConfig.hevc.lcuSize) ? mhw::sfc::LCU_32_32_HEVC
                                                                                                                                                     : mhw::sfc::LCU_64_64_HEVC;
    }
    else if (CODECHAL_VP9 == m_videoConfig.codecStandard)
    {
        VPHAL_COLORPACK colorPack = VpHalDDIUtils::GetSurfaceColorPack(m_renderData.SfcInputFormat);

        if ((VPHAL_COLORPACK_420 == colorPack)
            || (VPHAL_COLORPACK_444 == colorPack))
        {
            sfcStateParams->dwVDVEInputOrderingMode = mhw::sfc::LCU_64_64_VP9;
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::AddSfcLock(
    PMOS_COMMAND_BUFFER     pCmdBuffer,
    mhw::sfc::SFC_LOCK_PAR *pSfcLockParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_miItf);

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBase::AddSfcLock(
        pCmdBuffer,
        pSfcLockParams));

    //insert 2 dummy VD_CONTROL_STATE packets with data=0 after every HCP_SFC_LOCK
    if (mhw::sfc::SFC_PIPE_MODE_HCP == m_pipeMode && MEDIA_IS_WA(m_waTable, Wa_14010222001))
    {
        auto &vdCtrlParam          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdCtrlParam                = {};
        for (int i = 0; i < 2; i++)
        {
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(pCmdBuffer));
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::InitSfcStateParams()
{
    VP_FUNC_CALL();

    if (nullptr == m_sfcStateParams)
    {
        m_sfcStateParams = (mhw::sfc::SFC_STATE_PAR *)MOS_AllocAndZeroMemory(sizeof(mhw::sfc::SFC_STATE_PAR));
    }
    else
    {
        MOS_ZeroMemory(m_sfcStateParams, sizeof(mhw::sfc::SFC_STATE_PAR));
    }

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcStateParams);

    m_renderData.sfcStateParams = m_sfcStateParams;

    return MOS_STATUS_SUCCESS;
}

bool SfcRenderXe3_Lpm_Base::IsVdboxSfcInputFormatSupported(
    CODECHAL_STANDARD           codecStandard,
    MOS_FORMAT                  inputFormat)
{
    VP_FUNC_CALL();

    if (CODECHAL_JPEG == codecStandard)
    {
        if ((inputFormat != Format_NV12) &&
            (inputFormat != Format_400P) &&
            (inputFormat != Format_411P) &&
            (inputFormat != Format_IMC3) &&
            (inputFormat != Format_422H) &&
            (inputFormat != Format_444P) &&
            (inputFormat != Format_P010))
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Input Format '0x%08x' for SFC.", inputFormat);
            return false;
        }

        return true;
    }
    else if (CODECHAL_AV1 == codecStandard)
    {
        VPHAL_COLORPACK colorPack = VpHalDDIUtils::GetSurfaceColorPack(inputFormat);
        if (VPHAL_COLORPACK_420 != colorPack)
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Input Format '0x%08x' for SFC.", inputFormat);
            return false;
        }

        return true;
    }
    else
    {
        return SfcRenderBase::IsVdboxSfcInputFormatSupported(codecStandard, inputFormat);
    }
}

bool SfcRenderXe3_Lpm_Base::IsVdboxSfcOutputFormatSupported(
    CODECHAL_STANDARD           codecStandard,
    MOS_FORMAT                  outputFormat,
    MOS_TILE_TYPE               tileType)
{
    VP_FUNC_CALL();

    if (CODECHAL_JPEG == codecStandard)
    {
        if (outputFormat != Format_A8R8G8B8 &&
            outputFormat != Format_NV12 &&
            outputFormat != Format_P010 &&
            outputFormat != Format_YUY2)
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for SFC.", outputFormat);
            return false;
        }
        return true;
    }
    else if (CODECHAL_AV1 == codecStandard)
    {
        if ((outputFormat != Format_A8R8G8B8) &&
            (outputFormat != Format_NV12) &&
            (outputFormat != Format_P010) &&
            (outputFormat != Format_YUY2) &&
            (outputFormat != Format_AYUV) &&
            (outputFormat != Format_P016) &&
            (outputFormat != Format_Y210) &&
            (outputFormat != Format_Y216) &&
            (outputFormat != Format_Y410) &&
            (outputFormat != Format_Y416))
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for SFC.", outputFormat);
            return false;
        }

        return true;
    }
    else
    {
        return SfcRenderBase::IsVdboxSfcOutputFormatSupported(codecStandard, outputFormat, tileType);
    }
}

MOS_STATUS SfcRenderXe3_Lpm_Base::SetupScalabilityParams()
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_renderData.sfcStateParams);

    if (mhw::sfc::SFC_PIPE_MODE_HCP != m_pipeMode &&
        mhw::sfc::SFC_PIPE_MODE_VEBOX != m_pipeMode)
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

    m_renderData.sfcStateParams->engineMode = m_scalabilityParams.engineMode;

    if (mhw::sfc::SFC_PIPE_MODE_HCP == m_pipeMode)
    {
        VPHAL_COLORPACK colorPack = VpHalDDIUtils::GetSurfaceColorPack(m_renderData.SfcInputFormat);

        if ((VPHAL_COLORPACK_420 == colorPack || VPHAL_COLORPACK_422 == colorPack) &&
            (!MOS_IS_ALIGNED(m_scalabilityParams.srcStartX, 2) || MOS_IS_ALIGNED(m_scalabilityParams.srcEndX, 2)))
        {
            VP_PUBLIC_ASSERTMESSAGE("srcStartX(%d) is not even or srcEndX(%d) is not odd with input format(%d).",
                m_scalabilityParams.srcStartX, m_scalabilityParams.srcEndX, m_renderData.SfcInputFormat);
        }
        m_renderData.sfcStateParams->tileType     = m_scalabilityParams.tileType;
        m_renderData.sfcStateParams->srcStartX    = m_scalabilityParams.srcStartX;
        m_renderData.sfcStateParams->srcEndX      = m_scalabilityParams.srcEndX;
        m_renderData.sfcStateParams->dstStartX    = m_scalabilityParams.dstStartX;
        m_renderData.sfcStateParams->dstEndX      = m_scalabilityParams.dstEndX;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::AllocateResources()
{
    VP_FUNC_CALL();

    bool                            allocated;
    PVP_SURFACE                     pTarget;

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(m_sfcStateParams);
    VP_RENDER_CHK_NULL_RETURN(m_renderData.pSfcPipeOutSurface);

    allocated = false;
    pTarget   = m_renderData.pSfcPipeOutSurface;

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBase::AllocateResources());

    // Allocate bottom field surface for interleaved to field
    if (m_renderData.sfcStateParams->iScalingType == ISCALING_INTERLEAVED_TO_FIELD)
    {
        VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
                                    m_tempFieldSurface,
                                    "OutputBottomFieldSurface",
                                    pTarget->osSurface->Format,
                                    MOS_GFXRES_2D,
                                    pTarget->osSurface->TileType,
                                    pTarget->osSurface->dwWidth,
                                    pTarget->osSurface->dwHeight,
                                    pTarget->osSurface->bIsCompressed,
                                    pTarget->osSurface->CompressionMode,
                                    allocated));
        VP_RENDER_CHK_NULL_RETURN(m_tempFieldSurface);
        VP_RENDER_CHK_NULL_RETURN(m_tempFieldSurface->osSurface);

        m_renderData.sfcStateParams->tempFieldResource = &m_tempFieldSurface->osSurface->OsResource;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::FreeResources()
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_allocator);

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBase::FreeResources());

    // Free bottom field surface for interleaved to field
    m_allocator->DestroyVpSurface(m_tempFieldSurface);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::SetScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBase::SetScalingParams(scalingParams));
    VP_RENDER_CHK_STATUS_RETURN(SetInterlacedScalingParams(scalingParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SfcRenderXe3_Lpm_Base::SetInterlacedScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(scalingParams);
    VP_RENDER_CHK_NULL_RETURN(m_sfcStateParams);

    // Set interlaced scaling parameters
    if (scalingParams->interlacedScalingType != ISCALING_NONE)
    {
        m_sfcStateParams->dwOutputFrameWidth  = m_sfcStateParams->dwScaledRegionWidth;
        m_sfcStateParams->dwOutputFrameHeight = m_sfcStateParams->dwScaledRegionHeight;
    }
    m_sfcStateParams->iScalingType = scalingParams->interlacedScalingType;
    switch (scalingParams->interlacedScalingType)
    {
    case ISCALING_INTERLEAVED_TO_INTERLEAVED:
        m_sfcStateParams->inputFrameDataFormat  = FRAME_FORMAT_INTERLEAVED;
        m_sfcStateParams->outputFrameDataFormat = FRAME_FORMAT_INTERLEAVED;
        // bottom field scaling offset
        m_sfcStateParams->bottomFieldVerticalScalingOffset = MOS_UF_ROUND(1.0F / 2.0F * (1.0F / m_sfcStateParams->fAVSYScalingRatio - 1.0F));
        break;
    case ISCALING_INTERLEAVED_TO_FIELD:
        m_sfcStateParams->inputFrameDataFormat  = FRAME_FORMAT_INTERLEAVED;
        m_sfcStateParams->outputFrameDataFormat = FRAME_FORMAT_FIELD;
        m_sfcStateParams->outputSampleType      = scalingParams->dstSampleType;
        break;
    case ISCALING_FIELD_TO_INTERLEAVED:
        m_sfcStateParams->inputFrameDataFormat  = FRAME_FORMAT_FIELD;
        m_sfcStateParams->outputFrameDataFormat = FRAME_FORMAT_INTERLEAVED;
        if (scalingParams->srcSampleType == SAMPLE_SINGLE_TOP_FIELD)
        {
            m_sfcStateParams->topBottomField = VPHAL_TOP_FIELD;
            if (scalingParams->dstSampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)
            {
                m_sfcStateParams->topBottomFieldFirst = VPHAL_TOP_FIELD_FIRST;
            }
            else
            {
                m_sfcStateParams->topBottomFieldFirst = VPHAL_BOTTOM_FIELD_FIRST;
            }
        }
        else
        {
            m_sfcStateParams->topBottomField = VPHAL_BOTTOM_FIELD;
            if (scalingParams->dstSampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)
            {
                m_sfcStateParams->topBottomFieldFirst = VPHAL_TOP_FIELD_FIRST;
            }
            else
            {
                m_sfcStateParams->topBottomFieldFirst = VPHAL_BOTTOM_FIELD_FIRST;
            }
        }
        break;
    case ISCALING_FIELD_TO_FIELD:
    case ISCALING_NONE:
    default:
        m_sfcStateParams->inputFrameDataFormat  = FRAME_FORMAT_PROGRESSIVE;
        m_sfcStateParams->outputFrameDataFormat = FRAME_FORMAT_PROGRESSIVE;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

uint32_t SfcRenderXe3_Lpm_Base::GetSfdLineBufferSize(bool lineTiledBuffer, MOS_FORMAT formatOutput, uint32_t widthOutput, uint32_t heightOutput)
{
    VP_FUNC_CALL();

    int size = 0;  
    if (mhw::sfc::SFC_PIPE_MODE_AVP == m_pipeMode)
    {
        size = (widthOutput * SFC_SFD_LINEBUFFER_SIZE_PER_PIXEL) * 2;  //for both line and row
        if (lineTiledBuffer && size > 0)
        {
            size += (1024 + 64 + 8) * MHW_SFC_CACHELINE_SIZE; 
        }
    }
    else
    {
        size = SfcRenderBase::GetSfdLineBufferSize(lineTiledBuffer, formatOutput, widthOutput, heightOutput);
    }
    return size;
}

//!
//! \brief    Set sfc pipe selected with vebox
//! \details  Set sfc pipe selected with vebox
//! \param    [in] dwSfcIndex
//!           Sfc pipe selected with vebox
//! \param    [in] dwSfcCount
//!           Sfc pipe num in total
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
MOS_STATUS SfcRenderXe3_Lpm_Base::SetSfcPipe(
    uint32_t dwSfcPipe,
    uint32_t dwSfcNum)
{
    VP_FUNC_CALL();

    MOS_STATUS         eStatus = MOS_STATUS_SUCCESS;

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcItf);
    std::shared_ptr<mhw::sfc::Itf> sfcItf = m_sfcItf;

    if (dwSfcPipe >= dwSfcNum)
    {
        VP_PUBLIC_ASSERTMESSAGE("Scalability sfc pipe set by vebox, dwSfcPipe %d, dwSfcNum %d", dwSfcPipe, dwSfcNum);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_scalabilityParams.curPipe    = dwSfcPipe;
    m_scalabilityParams.numPipe    = dwSfcNum;
    m_scalabilityParams.engineMode = (0 == m_scalabilityParams.curPipe) ? 1 : (m_scalabilityParams.numPipe - 1 == m_scalabilityParams.curPipe) ? 2 : 3;

    sfcItf->SetSfcIndex(dwSfcPipe, dwSfcNum);

    return eStatus;
}

bool SfcRenderXe3_Lpm_Base::IsOutputChannelSwapNeeded(MOS_FORMAT outputFormat)
{
    VP_FUNC_CALL();

    // ARGB8,ABGR10,A16B16G16R16,BGRP,VYUY and YVYU output format need to enable swap
    if (outputFormat == Format_X8R8G8B8     ||
        outputFormat == Format_A8R8G8B8     ||
        outputFormat == Format_R10G10B10A2  ||
        outputFormat == Format_A16B16G16R16 ||
        outputFormat == Format_VYUY         ||
        outputFormat == Format_YVYU         ||
        outputFormat == Format_BGRP)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SfcRenderXe3_Lpm_Base::IsCscNeeded(SFC_CSC_PARAMS &cscParams)
{
    VP_FUNC_CALL();
    if (m_bVdboxToSfc && m_videoConfig.codecStandard == CODECHAL_JPEG)
    {
        if (cscParams.inputFormat != cscParams.outputFormat)
        {
            return true;
        }
    }
    return cscParams.bCSCEnabled || IsInputChannelSwapNeeded(cscParams.inputFormat);
}

MOS_STATUS SfcRenderXe3_Lpm_Base::AddSfcState(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    mhw::sfc::SFC_STATE_PAR        *pSfcState,
    PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface)
{
    VP_FUNC_CALL();
    int32_t i = 0;

    MHW_RENDERHAL_CHK_NULL_RETURN(pSfcState);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_sfcItf);

    auto& params = m_sfcItf->MHW_GETPAR_F(SFC_STATE)();
    params = {};
    params = *pSfcState;
    params.pOutSurface = pOutSurface;

    // Send SFC_LOCK command to acquire SFC pipe for Vebox
    VP_RENDER_CHK_STATUS_RETURN(m_sfcItf->MHW_ADDCMD_F(SFC_STATE)(pCmdBuffer));

    return MOS_STATUS_SUCCESS;
}

