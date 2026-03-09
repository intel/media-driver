/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     mhw_sfc_xe3_lpm_base_impl.h
//! \brief    MHW sfc interface common base for Xe3_LPM
//! \details
//!

#ifndef __MHW_SFC_XE3_LPM_BASE_IMPL_H__
#define __MHW_SFC_XE3_LPM_BASE_IMPL_H__

#include "mhw_sfc_impl.h"
#include "mhw_sfc_hwcmd_xe3_lpm_base.h"
#include "mhw_sfc_itf.h"
#include "mhw_impl.h"
#include "vp_common.h"
#include "vp_utils.h"
#include "vp_hal_ddi_utils.h"
#include "mhw_sfc_common_impl.h"

namespace mhw
{
namespace sfc
{
namespace xe3_lpm_base
{
class Impl : public sfc::Impl<mhw::sfc::xe3_lpm_base::Cmd>
{
public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf)
    {
        MHW_FUNCTION_ENTER;
    };

    MOS_STATUS SetOutputSurfaceFormatType(
        mhw::sfc::xe3_lpm_base::Cmd::SFC_STATE_CMD *cmd,
        SFC_STATE_PAR                          params,
        PMHW_SFC_OUT_SURFACE_PARAMS            pOutSurface,
        bool                                   &bInterleaveChroma,
        uint16_t                               &wUYOffset)
    {
        MHW_CHK_NULL_RETURN(cmd);
        MHW_CHK_NULL_RETURN(pOutSurface);
        switch (params.OutputFrameFormat)
        {
        case Format_RGBP:
        case Format_BGRP:
            cmd->DW3.RgbPlanarMemoryFormatEnable = 1;
            cmd->DW3.OutputSurfaceFormatType     = cmd->OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R8;
            break;
        case Format_R8G8B8:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_INTEGRAL_32;
            break;
        case Format_AYUV:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_AYUV;
            break;
        case Format_X8R8G8B8:
        case Format_A8R8G8B8:
        case Format_X8B8G8R8:
        case Format_A8B8G8R8:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R8;
            break;
        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_A2R10G10B10;
            break;
        case Format_R5G6B5:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_R5G6B5;
            break;
        case Format_NV12:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_NV12;
            bInterleaveChroma                = true;
            wUYOffset                        = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_YVYU:
        case Format_YUY2:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_YUYV;
            break;
        case Format_VYUY:
        case Format_UYVY:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_UYVY;
            break;
        case Format_P010:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_P016;
            cmd->DW4.Bitdepth                = 0;
            wUYOffset                        = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_P016:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_P016;
            cmd->DW4.Bitdepth                = 1;
            wUYOffset                        = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_Y210:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_Y216;
            cmd->DW4.Bitdepth                = 0;
            wUYOffset                        = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_Y216:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_Y216;
            cmd->DW4.Bitdepth                = 1;
            wUYOffset                        = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_Y410:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_Y416;
            cmd->DW4.Bitdepth                = 0;
            wUYOffset                        = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_Y416:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_Y416;
            cmd->DW4.Bitdepth                = 1;
            wUYOffset                        = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_Y8:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R83;
            cmd->DW4.Bitdepth                = 0;
            break;
        case Format_Y16U:
        case Format_Y16S:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R84;
            cmd->DW4.Bitdepth                = 0;
            break;
        case Format_A16R16G16B16:
        case Format_A16B16G16R16:
            cmd->DW3.OutputSurfaceFormatType = cmd->OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R85;
            cmd->DW4.Bitdepth                = 0;
            break;
        default:
            MHW_ASSERTMESSAGE("Unknown Output Format.");
            return MOS_STATUS_UNKNOWN;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetOutputFrameSurface(
        mhw::sfc::xe3_lpm_base::Cmd::SFC_STATE_CMD *cmd,
        SFC_STATE_PAR                          params,
        PMHW_SFC_OUT_SURFACE_PARAMS            pOutSurface)
    {
        MHW_CHK_NULL_RETURN(cmd);
        MHW_CHK_NULL_RETURN(pOutSurface);
        // Set DW19
        if (params.bMMCEnable &&
            (params.MMCMode == MOS_MMC_RC ||
                params.MMCMode == MOS_MMC_MC))
        {
            cmd->DW3.OutputCompressionFormat                               = pOutSurface->dwCompressionFormat;
        }

        // copy from base surface
        cmd->DW57.BottomFieldSurfaceBaseAddressArbitrationPriorityControl =
            cmd->DW19.OutputFrameSurfaceBaseAddressArbitrationPriorityControl;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetOutputSurfaceOffset(
        mhw::sfc::xe3_lpm_base::Cmd::SFC_STATE_CMD *cmd,
        uint16_t                               wUYOffset,
        uint16_t                               wUXOffset,
        uint16_t                               wVYOffset,
        uint16_t                               wVXOffset,
        SFC_STATE_PAR                          params,
        PMHW_SFC_OUT_SURFACE_PARAMS            pOutSurface)
    {
        MHW_CHK_NULL_RETURN(cmd);
        MHW_CHK_NULL_RETURN(pOutSurface);
        // Set DW30, DW31
        cmd->DW30.OutputSurfaceYOffsetForU = wUYOffset;
        cmd->DW30.OutputSurfaceXOffsetForU = wUXOffset;
        cmd->DW31.OutputSurfaceYOffsetForV = wVYOffset;
        cmd->DW31.OutputSurfaceXOffsetForV = wVXOffset;

        if (Format_RGBP == params.OutputFrameFormat || Format_BGRP == params.OutputFrameFormat)
        {
            cmd->DW30.OutputSurfaceYOffsetForU = (uint16_t)pOutSurface->dwUYoffset;
            cmd->DW31.OutputSurfaceYOffsetForV = (uint16_t)pOutSurface->dwVUoffset + (uint16_t)pOutSurface->dwUYoffset;
        }
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_LOCK)
    {
        _MHW_SETCMD_CALLBASE(SFC_LOCK);

        //MHW_CHK_NULL_RETURN(cmd.cmdPtr);
        if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_HCP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
        }
        else if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_AVP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE;
        }
        else
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMISC;
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_STATE)
    {
        _MHW_SETCMD_CALLBASE(SFC_STATE);

        using cmd_t = mhw::sfc::xe3_lpm_base::Cmd;

        bool                         bHalfPitchForChroma = false;
        bool                         bInterleaveChroma   = false;
        uint16_t                     wUXOffset           = 0;
        uint16_t                     wUYOffset           = 0;
        uint16_t                     wVXOffset           = 0;
        uint16_t                     wVYOffset           = 0;
        MHW_RESOURCE_PARAMS          resourceParams;
        MEDIA_WA_TABLE              *pWaTable     = nullptr;
        PMOS_INTERFACE               pOsInterface = nullptr;
        PMHW_SFC_OUT_SURFACE_PARAMS  pOutSurface  = nullptr;

        MHW_CHK_NULL_RETURN(this->m_currentCmdBuf);
        pOutSurface = params.pOutSurface;
        MHW_CHK_NULL_RETURN(pOutSurface);

        pOsInterface = m_osItf;
        MHW_CHK_NULL_RETURN(m_osItf);
        pWaTable = pOsInterface->pfnGetWaTable(pOsInterface);
        MHW_CHK_NULL_RETURN(pWaTable);

        // Check input/output size
        MHW_ASSERT(params.dwInputFrameWidth  >= m_minWidth);
        MHW_ASSERT(params.dwInputFrameHeight >= m_minHeight);
        MHW_ASSERT(params.dwOutputFrameWidth  <= m_maxWidth);
        MHW_ASSERT(params.dwOutputFrameHeight <= m_maxHeight);

        // Set output surface format type (member function, unchanged)
        // RGBASwapEnable is true when the OutputSurfaceFormatType is set as A8B8G8R8 for X8R8G8B8 and A8R8G8B8 output,
        // the OutputSurfaceFormatType is set as A2R10G10B10 for R10G10B10A2 output,
        // the OutputSurfaceFormatType is set as YUYV for YVYU output,
        // the OutputSurfaceFormatType is set as UYVY for VYUY output and
        // the OutputSurfaceFormatType is set as A8B8G8R84 for A16R16G16B16 output.
        bool     &refInterleaveChroma = bInterleaveChroma;
        uint16_t &refUYOffset         = wUYOffset;
        SetOutputSurfaceFormatType(&cmd, params, pOutSurface, refInterleaveChroma, refUYOffset);

        // Common basic fields: DW0-DW9, DW14-DW15, DW33-DW37, DW1 frame format, DW54, DW1 TopBottom
        mhw::sfc::common::SetSfcStateBasicFields<cmd_t>(cmd, params, pOutSurface, m_outputCenteringEnable);

        // Gray bar pixels: U10 fixed-point (Group B: no FP16)
        mhw::sfc::common::SetSfcStateGrayBarPixelsU10<cmd_t>(cmd, params);

        // Set output frame surface (member function, unchanged)
        SetOutputFrameSurface(&cmd, params, pOutSurface);

        // DW29: tile mode, pitch, interleave chroma, format
        mhw::sfc::common::SetSfcStateSurfaceParams<cmd_t>(
            cmd, pOutSurface, bHalfPitchForChroma, bInterleaveChroma,
            MosGetHWTileType(pOutSurface->TileType, pOutSurface->TileModeGMM, pOutSurface->bGMMTileEnabled));

        // DW30-DW32: output surface UV offsets (member function, unchanged)
        SetOutputSurfaceOffset(&cmd, wUYOffset, wUXOffset, wVYOffset, wVXOffset, params, pOutSurface);

        // Resource bindings: DW17-DW60
        auto addResFn = [&](PMOS_INTERFACE pOsItf, PMOS_COMMAND_BUFFER pCmdBuf, MHW_RESOURCE_PARAMS *pResParams) -> MOS_STATUS {
            return AddResourceToCmd(pOsItf, pCmdBuf, pResParams);
        };
        auto initMocsFn = [&](MHW_RESOURCE_PARAMS &resParams, uint32_t *pCmd, uint8_t startBit, uint8_t endBit) {
            InitMocsParams(resParams, pCmd, startBit, endBit);
        };
        MHW_CHK_STATUS_RETURN(mhw::sfc::common::SetSfcStateResourceBindings<cmd_t>(
            cmd, params, pOsInterface, m_currentCmdBuf,
            m_outputSurfCtrl, m_avsLineBufferCtrl, m_iefLineBufferCtrl, m_sfdLineBufferCtrl,
            m_avsLineTileBufferCtrl, m_iefLineTileBufferCtrl, m_sfdLineTileBufferCtrl, m_histogramBufferCtrl,
            addResFn, initMocsFn));

        // Dithering: DW3 + DW50-DW53
        mhw::sfc::common::SetSfcStateDithering<cmd_t>(cmd, params);

        // Scalability block (Group B: standard MHW_SFC_MAX_PIPE_NUM / MHW_SFC_INDEX_0/1/2)
        mhw::sfc::common::SetSfcStateScalability<cmd_t>(
            cmd, params,
            m_sfcScalabilityEnabled, m_numofSfc, m_indexofSfc,
            MHW_SFC_MAX_PIPE_NUM,
            MHW_SFC_INDEX_0,
            MHW_SFC_INDEX_1,
            MHW_SFC_INDEX_2);

        // Group B: rectangle block included
        mhw::sfc::common::SetSfcStateRectangle<cmd_t>(cmd, params);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_AVS_STATE)
    {
        _MHW_SETCMD_CALLBASE(SFC_AVS_STATE);

        //MHW_CHK_NULL_RETURN(cmdPtr);
        if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_HCP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
        }
        else if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_AVP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE;
        }
        else
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMISC;
        }

        cmd.DW3.InputHorizontalSitingValueSpecifiesTheHorizontalSitingOfTheInput = params.dwInputHorizontalSiting;
        cmd.DW3.InputVerticalSitingSpecifiesTheVerticalSitingOfTheInput          = params.dwInputVerticalSitting;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_FRAME_START)
    {
        _MHW_SETCMD_CALLBASE(SFC_FRAME_START);

        //MHW_CHK_NULL_RETURN(cmdPtr);
        if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_HCP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
        }
        else if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_AVP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE;
        }
        else
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMISC;
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_IEF_STATE)
    {
        _MHW_SETCMD_CALLBASE(SFC_IEF_STATE);

        //MHW_CHK_NULL_RETURN(cmdPtr);
        if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_HCP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
        }
        else if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_AVP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE;
        }
        else
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMISC;
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_AVS_CHROMA_Coeff_Table)
    {
        _MHW_SETCMD_CALLBASE(SFC_AVS_CHROMA_Coeff_Table);

        //MHW_CHK_NULL_RETURN(cmdPtr);
        if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_HCP)  // HCP-to-SFC
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
        }
        else if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_AVP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE;
        }
        else
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMISC;
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(SFC_AVS_LUMA_Coeff_Table)
    {
        _MHW_SETCMD_CALLBASE(SFC_AVS_LUMA_Coeff_Table);

        //MHW_CHK_NULL_RETURN(cmdPtr);
        if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_HCP)  // HCP-to-SFC
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
        }
        else if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_AVP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE;
        }
        else
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMISC;
        }

        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t = sfc::Impl<mhw::sfc::xe3_lpm_base::Cmd>;

MEDIA_CLASS_DEFINE_END(mhw__sfc__xe3_lpm_base__Impl)
};

}  // namespace xe3_lpm_base
}  // namespace sfc
}  // namespace mhw

#endif  // __MHW_SFC_XE3_LPM_BASE_IMPL_H__
