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

#define VALUE_XOFFSET 3
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

        bool                                   bHalfPitchForChroma;
        bool                                   bInterleaveChroma;
        uint16_t                               wUXOffset;
        uint16_t                               wUYOffset;
        uint16_t                               wVXOffset;
        uint16_t                               wVYOffset;
        MHW_RESOURCE_PARAMS                    resourceParams;
        MEDIA_WA_TABLE *                       pWaTable = nullptr;
        PMOS_INTERFACE                         pOsInterface = nullptr;
        PMHW_SFC_OUT_SURFACE_PARAMS            pOutSurface  = nullptr;

        MHW_CHK_NULL_RETURN(this->m_currentCmdBuf);
        pOutSurface = params.pOutSurface;
        MHW_CHK_NULL_RETURN(pOutSurface);

        pOsInterface = m_osItf;
        MHW_CHK_NULL_RETURN(m_osItf);
        pWaTable = pOsInterface->pfnGetWaTable(pOsInterface);
        MHW_CHK_NULL_RETURN(pWaTable);

        bHalfPitchForChroma = false;
        bInterleaveChroma   = false;
        wUXOffset           = 0;
        wUYOffset           = 0;
        wVXOffset           = 0;
        wVYOffset           = 0;

        // Check input/output size
        MHW_ASSERT(params.dwInputFrameWidth >= m_minWidth);
        MHW_ASSERT(params.dwInputFrameHeight >= m_minHeight);
        MHW_ASSERT(params.dwOutputFrameWidth <= m_maxWidth);
        MHW_ASSERT(params.dwOutputFrameHeight <= m_maxHeight);

        // Set DW0
        if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_HCP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHCPSFCMODE;
        }
        else if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_AVP)
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE;
        }
        else
        {
            cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
        }

        // Set DW1
        cmd.DW1.SfcPipeMode               = params.sfcPipeMode;
        cmd.DW1.SfcInputChromaSubSampling = params.dwInputChromaSubSampling;
        cmd.DW1.VdVeInputOrderingMode     = params.dwVDVEInputOrderingMode;
        cmd.DW1.SfcEngineMode             = params.engineMode;

        // Set DW2
        cmd.DW2.InputFrameResolutionWidth  = params.dwInputFrameWidth - 1;
        cmd.DW2.InputFrameResolutionHeight = params.dwInputFrameHeight - 1;

        // Set DW3
        cmd.DW3.OutputChromaDownsamplingCoSitingPositionVerticalDirection   = params.dwChromaDownSamplingVerticalCoef;
        cmd.DW3.OutputChromaDownsamplingCoSitingPositionHorizontalDirection = params.dwChromaDownSamplingHorizontalCoef;
        cmd.DW3.InputColorSpace0Yuv1Rgb                                     = params.bInputColorSpace;

        bool     &refInterleaveChroma = bInterleaveChroma;
        uint16_t &refUYOffset         = wUYOffset;
        SetOutputSurfaceFormatType(&cmd, params, pOutSurface, refInterleaveChroma, refUYOffset);
        // RGBASwapEnable is true when the OutputSurfaceFormatType is set as A8B8G8R8 for X8R8G8B8 and A8R8G8B8 output,
        // the OutputSurfaceFormatType is set as A2R10G10B10 for R10G10B10A2 output,
        // the OutputSurfaceFormatType is set as YUYV for YVYU output,
        // the OutputSurfaceFormatType is set as UYVY for VYUY output and
        // the OutputSurfaceFormatType is set as A8B8G8R84 for A16R16G16B16 output.
        cmd.DW3.ChannelSwapEnable = params.bRGBASwapEnable;
        // Set DW4
        cmd.DW4.IefEnable              = params.bIEFEnable;
        cmd.DW4.SkinToneTunedIefEnable = params.bSkinToneTunedIEFEnable;
        cmd.DW4.AvsFilterMode          = params.dwAVSFilterMode;
        if (params.b8tapChromafiltering)
        {
            cmd.DW4.AdaptiveFilterForAllChannels = true;
        }
        else
        {
            cmd.DW4.AdaptiveFilterForAllChannels = false;
        }

        cmd.DW4.AvsScalingEnable = ((params.fAVSXScalingRatio == 1.0F) &&
                                       (params.fAVSYScalingRatio == 1.0F))
                                       ? false
                                       : true;
        cmd.DW4.BypassYAdaptiveFiltering             = params.bBypassYAdaptiveFilter;
        cmd.DW4.BypassXAdaptiveFiltering             = params.bBypassXAdaptiveFilter;
        cmd.DW4.ChromaUpsamplingEnable               = params.bAVSChromaUpsamplingEnable;
        cmd.DW4.RotationMode                         = params.RotationMode;
        cmd.DW4.ColorFillEnable                      = params.bColorFillEnable;
        cmd.DW4.CscEnable                            = params.bCSCEnable;
        cmd.DW4.Enable8TapForChromaChannelsFiltering = params.b8tapChromafiltering;
        cmd.DW4.TileType                             = params.tileType;
        cmd.DW4.RgbAdaptive                          = params.bRGBAdaptive;

        if (params.bMirrorEnable)
        {
            cmd.DW4.Value |= (uint32_t)(1 << 14) & 0x00004000;                              // Mirror Mode
            cmd.DW4.Value |= (uint32_t)(params.dwMirrorType << 13) & 0x00002000;  // Mirror Type
        }

        // Set DW5, DW6, DW7, DW8, DW9
        cmd.DW5.SourceRegionWidth            = params.dwSourceRegionWidth - 1;
        cmd.DW5.SourceRegionHeight           = params.dwSourceRegionHeight - 1;
        cmd.DW6.SourceRegionHorizontalOffset = params.dwSourceRegionHorizontalOffset;
        cmd.DW6.SourceRegionVerticalOffset   = params.dwSourceRegionVerticalOffset;
        cmd.DW7.OutputFrameWidth             = params.dwOutputFrameWidth + pOutSurface->dwSurfaceXOffset - 1;
        cmd.DW7.OutputFrameHeight            = params.dwOutputFrameHeight + pOutSurface->dwSurfaceYOffset - 1;
        cmd.DW8.ScaledRegionSizeWidth        = params.dwScaledRegionWidth - 1;
        cmd.DW8.ScaledRegionSizeHeight       = params.dwScaledRegionHeight - 1;
        cmd.DW9.ScaledRegionHorizontalOffset = params.dwScaledRegionHorizontalOffset + pOutSurface->dwSurfaceXOffset;
        cmd.DW9.ScaledRegionVerticalOffset   = params.dwScaledRegionVerticalOffset + pOutSurface->dwSurfaceYOffset;

        // Set DW10
        cmd.DW10.GrayBarPixelUG = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fColorFillUGPixel * 1024.0F), 0, 1023);  // U10
        cmd.DW10.GrayBarPixelYR = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fColorFillYRPixel * 1024.0F), 0, 1023);  // U10

        // Set DW11
        cmd.DW11.GrayBarPixelA  = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fColorFillAPixel * 1024.0F), 0, 1023);   // U10
        cmd.DW11.GrayBarPixelVB = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fColorFillVBPixel * 1024.0F), 0, 1023);  // U10

        // Set DW13
        cmd.DW13.AlphaDefaultValue = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fAlphaPixel * 1024.0F), 0, 1023);  // U10

        // Set DW14
        cmd.DW14.ScalingFactorHeight = (uint32_t)((double)params.dwSourceRegionHeight / (double)params.dwScaledRegionHeight * 524288.0F);  // U4.19

        // Set DW15
        cmd.DW15.ScaleFactorWidth = (uint32_t)((double)params.dwSourceRegionWidth / (double)params.dwScaledRegionWidth * 524288.0F);  // U4.19

        SetOutputFrameSurface(&cmd, params, pOutSurface);

        // Set DW29
        cmd.DW29.TiledMode                           = MosGetHWTileType(pOutSurface->TileType, pOutSurface->TileModeGMM, pOutSurface->bGMMTileEnabled);
        cmd.DW29.OutputSurfaceHalfPitchForChroma     = bHalfPitchForChroma;
        cmd.DW29.OutputSurfacePitch                  = pOutSurface->dwPitch - 1;
        cmd.DW29.OutputSurfaceInterleaveChromaEnable = bInterleaveChroma;
        cmd.DW29.OutputSurfaceFormat                 = cmd.DW3.OutputSurfaceFormatType;

        SetOutputSurfaceOffset(&cmd, wUYOffset, wUXOffset, wVYOffset, wVXOffset, params, pOutSurface);

        // Set DW33
        if (params.sfcPipeMode == SFC_PIPE_MODE::SFC_PIPE_MODE_AVP)
        {
            cmd.DW33.Av1TileColumnNumber = params.av1TileColumnNumber;
            cmd.DW33.Av1TileRowNumber    = params.av1TileRowNumber;
        }
        else
        {
            cmd.DW33.Av1TileColumnNumber = 0;
            cmd.DW33.Av1TileRowNumber    = 0;
        }

        // DW34, DW35
        cmd.DW34.Sourcestartx      = params.srcStartX;
        cmd.DW34.Sourceendx        = params.srcEndX;
        cmd.DW35.Destinationstartx = params.dstStartX;
        cmd.DW35.Destinationendx   = params.dstEndX;

        // Set DW36, DW37
        //Change SFC outputcentering scaling X/Yphaseshift value and limition limitione with 19bit following Fuslim setting.
        if (m_outputCenteringEnable)
        {
            cmd.DW36.Xphaseshift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)cmd.DW15.ScaleFactorWidth / 524288.0F - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
            cmd.DW37.Yphaseshift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)cmd.DW14.ScalingFactorHeight / 524288.0F - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
        }

        if (params.pOsResOutputSurface)
        {
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            if (params.iScalingType == ISCALING_INTERLEAVED_TO_FIELD &&
                params.outputSampleType == SAMPLE_SINGLE_BOTTOM_FIELD)
            {
                resourceParams.presResource    = params.tempFieldResource;
                resourceParams.pdwCmd          = &(cmd.DW17.Value);
                resourceParams.dwLocationInCmd = 17;
                resourceParams.HwCommandType   = MOS_SFC_STATE;
                resourceParams.bIsWritable     = true;
            }
            else
            {
                resourceParams.presResource    = params.pOsResOutputSurface;
                resourceParams.pdwCmd          = &(cmd.DW17.Value);
                resourceParams.dwLocationInCmd = 17;
                resourceParams.HwCommandType   = MOS_SFC_STATE;
                resourceParams.bIsWritable     = true;
            }
            resourceParams.dwOffset = params.dwOutputSurfaceOffset;
            InitMocsParams(resourceParams, &cmd.DW19.Value, 1, 6);
            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                m_currentCmdBuf,
                &resourceParams));
            if (m_outputSurfCtrl.Gen12_7.Index != 0)
            {
                cmd.DW19.OutputFrameSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables = m_outputSurfCtrl.Gen12_7.Index;
            }
        }

        if (params.pOsResAVSLineBuffer)
        {
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            resourceParams.presResource    = params.pOsResAVSLineBuffer;
            resourceParams.pdwCmd          = &(cmd.DW20.Value);
            resourceParams.dwLocationInCmd = 20;
            resourceParams.HwCommandType   = MOS_SFC_STATE;
            resourceParams.bIsWritable     = true;
            InitMocsParams(resourceParams, &cmd.DW22.Value, 1, 6);
            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                m_currentCmdBuf,
                &resourceParams));
            // Set DW22
            if (m_avsLineBufferCtrl.Gen12_7.Index != 0)
            {
                cmd.DW22.AvsLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_avsLineBufferCtrl.Gen12_7.Index;
            }
        }

        if (params.pOsResIEFLineBuffer)
        {
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            resourceParams.presResource    = params.pOsResIEFLineBuffer;
            resourceParams.pdwCmd          = &(cmd.DW23.Value);
            resourceParams.dwLocationInCmd = 23;
            resourceParams.HwCommandType   = MOS_SFC_STATE;
            resourceParams.bIsWritable     = true;
            InitMocsParams(resourceParams, &cmd.DW25.Value, 1, 6);
            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                m_currentCmdBuf,
                &resourceParams));
            // Set DW25
            if (m_iefLineBufferCtrl.Gen12_7.Index != 0)
            {
                cmd.DW25.IefLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_iefLineBufferCtrl.Gen12_7.Index;
            }
        }

        if (params.resSfdLineBuffer && !Mos_ResourceIsNull(params.resSfdLineBuffer))
        {
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            resourceParams.presResource    = params.resSfdLineBuffer;
            resourceParams.pdwCmd          = &(cmd.DW26.Value);
            resourceParams.dwLocationInCmd = 26;
            resourceParams.HwCommandType   = MOS_SFC_STATE;
            resourceParams.bIsWritable     = true;
            InitMocsParams(resourceParams, &cmd.DW28.Value, 1, 6);
            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                m_currentCmdBuf,
                &resourceParams));
            if (m_sfdLineBufferCtrl.Gen12_7.Index != 0)
            {
                cmd.DW28.SfdLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_sfdLineBufferCtrl.Gen12_7.Index;
            }
        }

        if (params.resAvsLineTileBuffer && !Mos_ResourceIsNull(params.resAvsLineTileBuffer))
        {
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            resourceParams.presResource    = params.resAvsLineTileBuffer;
            resourceParams.pdwCmd          = &(cmd.DW38.Value);
            resourceParams.dwLocationInCmd = 38;
            resourceParams.HwCommandType   = MOS_SFC_STATE;
            resourceParams.bIsWritable     = true;
            InitMocsParams(resourceParams, &cmd.DW40.Value, 1, 6);
            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                m_currentCmdBuf,
                &resourceParams));
            if (m_avsLineTileBufferCtrl.Gen12_7.Index != 0)
            {
                cmd.DW40.AvsLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_avsLineTileBufferCtrl.Gen12_7.Index;
            }
        }

        if (params.resIefLineTileBuffer && !Mos_ResourceIsNull(params.resIefLineTileBuffer))
        {
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            resourceParams.presResource    = params.resIefLineTileBuffer;
            resourceParams.pdwCmd          = &(cmd.DW41.Value);
            resourceParams.dwLocationInCmd = 41;
            resourceParams.HwCommandType   = MOS_SFC_STATE;
            resourceParams.bIsWritable     = true;
            InitMocsParams(resourceParams, &cmd.DW43.Value, 1, 6);
            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                m_currentCmdBuf,
                &resourceParams));
            if (m_iefLineTileBufferCtrl.Gen12_7.Index != 0)
            {
                cmd.DW43.IefLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_iefLineTileBufferCtrl.Gen12_7.Index;
            }
        }

        if (params.resSfdLineTileBuffer && !Mos_ResourceIsNull(params.resSfdLineTileBuffer))
        {
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            resourceParams.presResource    = params.resSfdLineTileBuffer;
            resourceParams.pdwCmd          = &(cmd.DW44.Value);
            resourceParams.dwLocationInCmd = 44;
            resourceParams.HwCommandType   = MOS_SFC_STATE;
            resourceParams.bIsWritable     = true;
            InitMocsParams(resourceParams, &cmd.DW46.Value, 1, 6);
            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                m_currentCmdBuf,
                &resourceParams));
            if (m_sfdLineTileBufferCtrl.Gen12_7.Index != 0)
            {
                cmd.DW46.SfdLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_sfdLineTileBufferCtrl.Gen12_7.Index;
            }
        }

        if (params.histogramSurface && !Mos_ResourceIsNull(&params.histogramSurface->OsResource))
        {
            cmd.DW4.HistogramStreamout = 1;
            cmd.DW49.Value             = 0;

            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            resourceParams.presResource    = &params.histogramSurface->OsResource;
            resourceParams.dwOffset        = params.histogramSurface->dwOffset;
            resourceParams.pdwCmd          = &(cmd.DW47.Value);
            resourceParams.dwLocationInCmd = 47;
            resourceParams.HwCommandType   = MOS_SFC_STATE;
            resourceParams.bIsWritable     = true;
            InitMocsParams(resourceParams, &cmd.DW49.Value, 1, 6);
            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                m_currentCmdBuf,
                &resourceParams));
            if (m_histogramBufferCtrl.Gen12_7.Index != 0)
            {
                cmd.DW49.HisgotramBaseAddressIndexToMemoryObjectControlStateMocsTables = m_histogramBufferCtrl.Gen12_7.Index;
            }
        }

        // Input/Output frame format
        cmd.DW1.InputFrameDataFormat  = params.inputFrameDataFormat;
        cmd.DW1.OutputFrameDataFormat = params.outputFrameDataFormat;

        // interleaved to interleaved
        if (params.iScalingType == ISCALING_INTERLEAVED_TO_INTERLEAVED)
        {
            cmd.DW54.BottomFieldVerticalScalingOffset = params.bottomFieldVerticalScalingOffset;
        }

        // Input is field mode
        if (params.iScalingType == ISCALING_FIELD_TO_INTERLEAVED)
        {
            cmd.DW1.TopBottomField      = params.topBottomField;
            cmd.DW1.TopBottomFieldFirst = params.topBottomFieldFirst;
        }

        // interleaved to field
        if (params.iScalingType == ISCALING_INTERLEAVED_TO_FIELD)
        {
            // Add bottom field address to cmd
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            if (params.outputSampleType == SAMPLE_SINGLE_BOTTOM_FIELD)
            {
                resourceParams.dwLsbNum        = 12;
                resourceParams.presResource    = params.pOsResOutputSurface;
                resourceParams.pdwCmd          = &cmd.DW55.Value;
                resourceParams.dwLocationInCmd = 55;
                resourceParams.bIsWritable     = true;
            }
            else
            {
                resourceParams.dwLsbNum        = 12;
                resourceParams.presResource    = params.tempFieldResource;
                resourceParams.pdwCmd          = &cmd.DW55.Value;
                resourceParams.dwLocationInCmd = 55;
                resourceParams.bIsWritable     = true;
            }
            InitMocsParams(resourceParams, &cmd.DW57.Value, 1, 6);
            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                m_currentCmdBuf,
                &resourceParams));
            if (m_outputSurfCtrl.Gen12_7.Index != 0)
            {
                cmd.DW57.BottomFieldSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables =
                    m_outputSurfCtrl.Gen12_7.Index;
            }

            cmd.DW58.BottomFieldSurfaceHalfPitchForChroma     = cmd.DW29.OutputSurfaceHalfPitchForChroma;
            cmd.DW58.BottomFieldSurfaceInterleaveChromaEnable = cmd.DW29.OutputSurfaceHalfPitchForChroma;
            cmd.DW58.BottomFieldSurfacePitch                  = cmd.DW29.OutputSurfacePitch;
            cmd.DW58.BottomFieldSurfaceTiled                  = (pOutSurface->TileType != MOS_TILE_LINEAR) ? true : false;
            cmd.DW58.BottomFieldSurfaceTileWalk               = (pOutSurface->TileType == MOS_TILE_Y) ? true : false;
            cmd.DW59.BottomFieldSurfaceXOffsetForU            = cmd.DW30.OutputSurfaceXOffsetForU;
            cmd.DW59.BottomFieldSurfaceYOffsetForU            = cmd.DW30.OutputSurfaceYOffsetForU;
            cmd.DW60.BottomFieldSurfaceXOffsetForV            = cmd.DW31.OutputSurfaceXOffsetForV;
            cmd.DW60.BottomFieldSurfaceYOffsetForV            = cmd.DW31.OutputSurfaceYOffsetForV;
        }

        // update dithering setting
        cmd.DW3.DitherEnable = params.ditheringEn;
        if (cmd.DW3.DitherEnable)
        {
            cmd.DW50.DitheringLutDelta12 = 0;
            cmd.DW50.DitheringLutDelta13 = 1;
            cmd.DW50.DitheringLutDelta14 = 1;
            cmd.DW50.DitheringLutDelta15 = 1;

            cmd.DW51.DitheringLutDelta8  = 0;
            cmd.DW51.DitheringLutDelta9  = 1;
            cmd.DW51.DitheringLutDelta10 = 1;
            cmd.DW51.DitheringLutDelta11 = 0;

            cmd.DW52.DitheringLutDelta4 = 0;
            cmd.DW52.DitheringLutDelta5 = 1;
            cmd.DW52.DitheringLutDelta6 = 0;
            cmd.DW52.DitheringLutDelta7 = 0;

            cmd.DW53.DitheringLutDelta0 = 0;
            cmd.DW53.DitheringLutDelta1 = 0;
            cmd.DW53.DitheringLutDelta2 = 0;
            cmd.DW53.DitheringLutDelta3 = 0;
        }

        if (m_sfcScalabilityEnabled == true)
        {
            uint32_t       iMediumX;
            uint32_t       xOffset;
            uint32_t       src_startX[MHW_SFC_MAX_PIPE_NUM];
            uint32_t       src_endX[MHW_SFC_MAX_PIPE_NUM];
            uint32_t       dest_startX[MHW_SFC_MAX_PIPE_NUM];
            uint32_t       dest_endX[MHW_SFC_MAX_PIPE_NUM];
            uint32_t       tile_endX;
            uint32_t       dest_first_valid_left_tile = 0;
            uint32_t       dest_last_valid_right_tile = m_numofSfc - 1;
            uint32_t       dest_cntX                  = 0;
            double         xLandingpoint;
            uint32_t       one_by_sf                    = (uint32_t)(((uint64_t)params.dwSourceRegionWidth * 524288L) / params.dwScaledRegionWidth);
            const uint32_t one_by_sf_fraction_precision = 19;
            const uint32_t beta_precision               = 5;
            int32_t        xPhaseShift;
            double         tempDestCntx;
            uint32_t       i;
            MHW_ASSERT(params.dwInputFrameWidth > m_numofSfc * 64);

            iMediumX = MOS_ALIGN_FLOOR((params.dwInputFrameWidth / m_numofSfc), 64);
            iMediumX = MOS_CLAMP_MIN_MAX(iMediumX, 64, (params.dwInputFrameWidth - 64));

            src_startX[0] = dest_startX[0] = 0;

            for (i = 0; i < m_numofSfc; i++)
            {
                if (i == m_numofSfc - 1)
                {
                    src_endX[i] = params.dwInputFrameWidth - 1;
                }
                else
                {
                    src_endX[i] = iMediumX * (i + 1) - 1;
                    if (params.dwInputFrameWidth != m_numofSfc * iMediumX)
                    {
                        src_endX[i] += 64;
                    }
                }

                if (params.dwSourceRegionHorizontalOffset + params.dwSourceRegionWidth - 1 <= src_endX[i])
                {
                    xOffset   = 0;
                    tile_endX = params.dwSourceRegionHorizontalOffset + params.dwSourceRegionWidth - 1;
                }
                else
                {
                    xOffset   = VALUE_XOFFSET;
                    tile_endX = src_endX[i];
                }

                while (1)
                {
                    if (src_endX[i] - params.dwSourceRegionHorizontalOffset < (xOffset + 1))
                    {
                        dest_endX[i] = 0;
                        break;
                    }

                    if (dest_cntX == 0)
                    {
                        dest_first_valid_left_tile = i;
                    }

                    xPhaseShift  = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)params.dwSourceRegionWidth / params.dwScaledRegionWidth - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
                    tempDestCntx = (((double)dest_cntX * (double)one_by_sf) + xPhaseShift);
                    if (tempDestCntx < 0)
                    {
                        tempDestCntx = 0;
                    }
                    xLandingpoint = (double)(((tempDestCntx + ((double)(1 << (one_by_sf_fraction_precision - beta_precision - 1)))) / 524288.0F) + params.dwSourceRegionHorizontalOffset);

                    if (xLandingpoint >= (double)(tile_endX - xOffset))
                    {
                        dest_endX[i] = dest_cntX - 1;
                        break;
                    }
                    else
                    {
                        dest_cntX++;
                    }

                    if (xOffset == 0)
                    {
                        dest_last_valid_right_tile = i;
                        dest_endX[i]               = params.dwScaledRegionWidth - 1;
                        break;
                    }
                }
            }

            for (i = 1; i < m_numofSfc; i++)
            {
                src_startX[i] = src_endX[i - 1] + 1 - 64;

                if (i <= dest_first_valid_left_tile)
                {
                    dest_startX[i] = 0;
                }
                else if (i <= dest_last_valid_right_tile)
                {
                    dest_startX[i] = dest_endX[i - 1] + 1;
                }
                else
                {
                    dest_startX[i] = 0;
                    dest_endX[i]   = 0;
                }
            }

            // Set SFC Engine Mode
            if (m_numofSfc == 2)
            {
                if (m_indexofSfc == MHW_SFC_INDEX_0)
                {
                    cmd.DW1.SfcEngineMode = 1;
                }
                else
                {
                    cmd.DW1.SfcEngineMode = 2;
                }
            }
            else if (m_numofSfc == 3)
            {
                if (m_indexofSfc == MHW_SFC_INDEX_0)
                {
                    cmd.DW1.SfcEngineMode = 1;
                }
                else if (m_indexofSfc == MHW_SFC_INDEX_1)
                {
                    cmd.DW1.SfcEngineMode = 3;
                }
                else
                {
                    cmd.DW1.SfcEngineMode = 2;
                }
            }
            else if (m_numofSfc == 4)
            {
                if (m_indexofSfc == MHW_SFC_INDEX_0)
                {
                    cmd.DW1.SfcEngineMode = 1;
                }
                else if (m_indexofSfc == MHW_SFC_INDEX_1 ||
                         m_indexofSfc == MHW_SFC_INDEX_2)
                {
                    cmd.DW1.SfcEngineMode = 3;
                }
                else
                {
                    cmd.DW1.SfcEngineMode = 2;
                }
            }

            if (m_indexofSfc < dest_first_valid_left_tile)
            {
                cmd.DW4.ColorFillEnable = 0;
            }
            else if (m_indexofSfc > dest_last_valid_right_tile)
            {
                cmd.DW4.ColorFillEnable = 0;
            }

            if (params.bIEFEnable)
            {
                if (dest_startX[m_indexofSfc] >= 4)
                {
                    dest_startX[m_indexofSfc] -= 4;
                }
                else
                {
                    dest_startX[m_indexofSfc] = 0;
                }
            }

            if (VpHalDDIUtils::GetSurfaceColorPack(params.OutputFrameFormat) != VPHAL_COLORPACK_444 &&
                (dest_startX[m_indexofSfc] % 2 != 0))
            {
                if (dest_startX[m_indexofSfc] >= 1)
                {
                    dest_startX[m_indexofSfc] -= 1;
                }
                else
                {
                    dest_startX[m_indexofSfc] = 0;
                }
            }

            cmd.DW34.Sourcestartx      = src_startX[m_indexofSfc];
            cmd.DW34.Sourceendx        = src_endX[m_indexofSfc];
            cmd.DW35.Destinationstartx = dest_startX[m_indexofSfc];
            cmd.DW35.Destinationendx   = dest_endX[m_indexofSfc];

            MHW_NORMALMESSAGE("SFC%d STATE: src startx %d endx %d", m_indexofSfc, cmd.DW34.Sourcestartx, cmd.DW34.Sourceendx);
            MHW_NORMALMESSAGE("SFC%d STATE: dest startx %d endx %d", m_indexofSfc, cmd.DW35.Destinationstartx, cmd.DW35.Destinationendx);
        }

    if (params.bRectangleEnabled)
    {
        cmd.DW61.TargetRectangleStartHorizontalOffset = params.dwTargetRectangleStartHorizontalOffset;
        cmd.DW61.TargetRectangleStartVerticalOffset   = params.dwTargetRectangleStartVerticalOffset;
        cmd.DW62.TargetRectangleEndHorizontalOffset   = params.dwTargetRectangleEndHorizontalOffset - 1;  // target rectangle end offsets are considered as zero based for hw
        cmd.DW62.TargetRectangleEndVerticalOffset     = params.dwTargetRectangleEndVerticalOffset - 1;    // target rectangle end offsets are considered as zero based for hw
    }
    else
    {
        cmd.DW61.TargetRectangleStartHorizontalOffset = 0;
        cmd.DW61.TargetRectangleStartVerticalOffset   = 0;
        cmd.DW62.TargetRectangleEndHorizontalOffset   = cmd.DW7.OutputFrameWidth;     // target rectangle end offsets are considered as zero based for hw
        cmd.DW62.TargetRectangleEndVerticalOffset     = cmd.DW7.OutputFrameHeight;    // target rectangle end offsets are considered as zero based for hw
    }
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
