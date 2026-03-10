// Copyright (c) 2026, Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//!
//! \file     mhw_sfc_common_impl.h
//! \brief    MHW SFC common non-member helper functions shared across platform implementations
//! \details  Provides templated helper functions in namespace mhw::sfc::common that
//!           encapsulate the shared logic of _MHW_SETCMD_OVERRIDE_DECL(SFC_STATE).
//!

#pragma once
#ifndef __MHW_SFC_COMMON_IMPL_H__
#define __MHW_SFC_COMMON_IMPL_H__

#include "mhw_sfc_cmdpar.h"
//#include "mhw_resource_params.h"
#include "mhw_utilities.h"
#include "mos_os.h"
#include "vp_hal_ddi_utils.h"

namespace mhw
{
namespace sfc
{
namespace common
{

#define VALUE_XOFFSET 3
//!
//! \brief  SetSfcStateBasicFields
//!         Sets DW0-DW9, DW14-DW15, DW33-DW37, DW1 frame format, DW54, DW1 TopBottom fields.
//!         Note: SetOutputSurfaceFormatType must be called BEFORE this helper to set DW3.OutputSurfaceFormatType.
//!         DW3.ChannelSwapEnable must be set BEFORE calling this helper as well.
//!
template <typename cmd_t>
inline void SetSfcStateBasicFields(
    typename cmd_t::SFC_STATE_CMD &cmd,
    const SFC_STATE_PAR    &params,
    PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface,
    bool                           outputCenteringEnable)
{
    // Set DW0: pipe mode opcode
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

    // Set DW1: SfcPipeMode / ChromaSubSampling / VdVeInputOrderingMode / SfcEngineMode
    cmd.DW1.SfcPipeMode               = params.sfcPipeMode;
    cmd.DW1.SfcInputChromaSubSampling = params.dwInputChromaSubSampling;
    cmd.DW1.VdVeInputOrderingMode     = params.dwVDVEInputOrderingMode;
    cmd.DW1.SfcEngineMode             = params.engineMode;

    // Set DW2: input frame resolution
    cmd.DW2.InputFrameResolutionWidth  = params.dwInputFrameWidth - 1;
    cmd.DW2.InputFrameResolutionHeight = params.dwInputFrameHeight - 1;

    // Set DW3: chroma downsampling + color space
    // Note: OutputSurfaceFormatType and ChannelSwapEnable are set by caller before this function
    cmd.DW3.OutputChromaDownsamplingCoSitingPositionVerticalDirection   = params.dwChromaDownSamplingVerticalCoef;
    cmd.DW3.OutputChromaDownsamplingCoSitingPositionHorizontalDirection = params.dwChromaDownSamplingHorizontalCoef;
    cmd.DW3.InputColorSpace0Yuv1Rgb                                     = params.bInputColorSpace;
    cmd.DW3.ChannelSwapEnable                                           = params.bRGBASwapEnable;

    // Set DW4: IEF/AVS/rotation/colorFill/CSC/mirror/tileType/RGBAdaptive
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
        cmd.DW4.Value |= (uint32_t)(1 << 14) & 0x00004000;                             // Mirror Mode
        cmd.DW4.Value |= (uint32_t)(params.dwMirrorType << 13) & 0x00002000;  // Mirror Type
    }

    // Set DW5, DW6, DW7, DW8, DW9: source/output/scaled region dimensions
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

    // Set DW14-DW15: scaling factors (round-to-zero cast)
    // Use round to zero for the scaling factor calculation to resolve tdr issue in scalability case
    cmd.DW14.ScalingFactorHeight = (uint32_t)((double)params.dwSourceRegionHeight / (double)params.dwScaledRegionHeight * 524288.0F);  // U4.19
    cmd.DW15.ScaleFactorWidth    = (uint32_t)((double)params.dwSourceRegionWidth  / (double)params.dwScaledRegionWidth  * 524288.0F);  // U4.19

    // Set DW33: AV1 tile numbers
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

    // DW34, DW35: initial src/dst start/end X from params
    cmd.DW34.Sourcestartx      = params.srcStartX;
    cmd.DW34.Sourceendx        = params.srcEndX;
    cmd.DW35.Destinationstartx = params.dstStartX;
    cmd.DW35.Destinationendx   = params.dstEndX;

    // Set DW36, DW37: phase shifts when output centering enabled
    // Change SFC outputcentering scaling X/Yphaseshift value and limitation with 19bit following Fuslim setting.
    if (outputCenteringEnable)
    {
        cmd.DW36.Xphaseshift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)cmd.DW15.ScaleFactorWidth / 524288.0F - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
        cmd.DW37.Yphaseshift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)cmd.DW14.ScalingFactorHeight / 524288.0F - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
    }

    // Input/Output frame format
    cmd.DW1.InputFrameDataFormat  = params.inputFrameDataFormat;
    cmd.DW1.OutputFrameDataFormat = params.outputFrameDataFormat;

    // DW54: interleaved-to-interleaved bottom field vertical scaling offset
    if (params.iScalingType == ISCALING_INTERLEAVED_TO_INTERLEAVED)
    {
        cmd.DW54.BottomFieldVerticalScalingOffset = params.bottomFieldVerticalScalingOffset;
    }

    // DW1: field-to-interleaved top/bottom field selection
    if (params.iScalingType == ISCALING_FIELD_TO_INTERLEAVED)
    {
        cmd.DW1.TopBottomField      = params.topBottomField;
        cmd.DW1.TopBottomFieldFirst = params.topBottomFieldFirst;
    }
}

//!
//! \brief  SetSfcStateGrayBarPixelsU10
//!         Sets DW10-DW11, DW13 gray bar pixel values using U10 fixed-point format.
//!         Used by Group A and B platforms (no FP16 support).
//!
template <typename cmd_t>
inline void SetSfcStateGrayBarPixelsU10(
    typename cmd_t::SFC_STATE_CMD &cmd,
    const SFC_STATE_PAR    &params)
{
    // Set DW10
    cmd.DW10.GrayBarPixelUG = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fColorFillUGPixel * 1024.0F), 0, 1023);  // U10
    cmd.DW10.GrayBarPixelYR = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fColorFillYRPixel * 1024.0F), 0, 1023);  // U10

    // Set DW11
    cmd.DW11.GrayBarPixelA  = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fColorFillAPixel  * 1024.0F), 0, 1023);  // U10
    cmd.DW11.GrayBarPixelVB = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fColorFillVBPixel * 1024.0F), 0, 1023);  // U10

    // Set DW13
    cmd.DW13.AlphaDefaultValue = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(params.fAlphaPixel * 1024.0F), 0, 1023);  // U10
}

//!
//! \brief  SetSfcStateSurfaceParams
//!         Sets DW29: tile mode, pitch, interleave chroma, format.
//!         Note: tiledMode is computed at call site to avoid this-> vs no-this-> divergence.
//!
template <typename cmd_t>
inline void SetSfcStateSurfaceParams(
    typename cmd_t::SFC_STATE_CMD &cmd,
    PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface,
    bool                           bHalfPitchForChroma,
    bool                           bInterleaveChroma,
    uint32_t               tiledMode)
{
    cmd.DW29.TiledMode                           = tiledMode;
    cmd.DW29.OutputSurfaceHalfPitchForChroma     = bHalfPitchForChroma;
    cmd.DW29.OutputSurfacePitch                  = pOutSurface->dwPitch - 1;
    cmd.DW29.OutputSurfaceInterleaveChromaEnable = bInterleaveChroma;
    cmd.DW29.OutputSurfaceFormat                 = cmd.DW3.OutputSurfaceFormatType;
}

//!
//! \brief  SetSfcStateResourceBindings
//!         Handles all conditional DW17-DW60 AddResourceToCmd calls via pass-through lambdas.
//!         Includes output surface, AVS/IEF/SFD line buffers, tile buffers, histogram, and
//!         the ISCALING_INTERLEAVED_TO_FIELD bottom field resource block.
//!
template <typename cmd_t, typename AddResFnT, typename InitMocsFnT>
inline MOS_STATUS SetSfcStateResourceBindings(
    typename cmd_t::SFC_STATE_CMD        &cmd,
    const SFC_STATE_PAR           &params,
    PMOS_INTERFACE                        pOsInterface,
    PMOS_COMMAND_BUFFER                   pCmdBuf,
    MHW_MEMORY_OBJECT_CONTROL_PARAMS      outputSurfCtrl,
    MHW_MEMORY_OBJECT_CONTROL_PARAMS      avsLineBufferCtrl,
    MHW_MEMORY_OBJECT_CONTROL_PARAMS      iefLineBufferCtrl,
    MHW_MEMORY_OBJECT_CONTROL_PARAMS      sfdLineBufferCtrl,
    MHW_MEMORY_OBJECT_CONTROL_PARAMS      avsLineTileBufferCtrl,
    MHW_MEMORY_OBJECT_CONTROL_PARAMS      iefLineTileBufferCtrl,
    MHW_MEMORY_OBJECT_CONTROL_PARAMS      sfdLineTileBufferCtrl,
    MHW_MEMORY_OBJECT_CONTROL_PARAMS      histogramBufferCtrl,
    AddResFnT                             addResourceFn,
    InitMocsFnT                           initMocsFn)
{
    MHW_RESOURCE_PARAMS resourceParams;

    // Block 1 (DW17-19): Output surface
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
        initMocsFn(resourceParams, &cmd.DW19.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(addResourceFn(pOsInterface, pCmdBuf, &resourceParams));
        if (outputSurfCtrl.Gen12_7.Index != 0)
        {
            cmd.DW19.OutputFrameSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables = outputSurfCtrl.Gen12_7.Index;
        }
    }

    // Block 2 (DW20-22): AVS line buffer
    if (params.pOsResAVSLineBuffer)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource    = params.pOsResAVSLineBuffer;
        resourceParams.pdwCmd          = &(cmd.DW20.Value);
        resourceParams.dwLocationInCmd = 20;
        resourceParams.HwCommandType   = MOS_SFC_STATE;
        resourceParams.bIsWritable     = true;
        initMocsFn(resourceParams, &cmd.DW22.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(addResourceFn(pOsInterface, pCmdBuf, &resourceParams));
        // Set DW22
        if (avsLineBufferCtrl.Gen12_7.Index != 0)
        {
            cmd.DW22.AvsLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = avsLineBufferCtrl.Gen12_7.Index;
        }
    }

    // Block 3 (DW23-25): IEF line buffer
    if (params.pOsResIEFLineBuffer)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource    = params.pOsResIEFLineBuffer;
        resourceParams.pdwCmd          = &(cmd.DW23.Value);
        resourceParams.dwLocationInCmd = 23;
        resourceParams.HwCommandType   = MOS_SFC_STATE;
        resourceParams.bIsWritable     = true;
        initMocsFn(resourceParams, &cmd.DW25.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(addResourceFn(pOsInterface, pCmdBuf, &resourceParams));
        // Set DW25
        if (iefLineBufferCtrl.Gen12_7.Index != 0)
        {
            cmd.DW25.IefLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = iefLineBufferCtrl.Gen12_7.Index;
        }
    }

    // Block 4 (DW26-28): SFD line buffer
    if (params.resSfdLineBuffer && !Mos_ResourceIsNull(params.resSfdLineBuffer))
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource    = params.resSfdLineBuffer;
        resourceParams.pdwCmd          = &(cmd.DW26.Value);
        resourceParams.dwLocationInCmd = 26;
        resourceParams.HwCommandType   = MOS_SFC_STATE;
        resourceParams.bIsWritable     = true;
        initMocsFn(resourceParams, &cmd.DW28.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(addResourceFn(pOsInterface, pCmdBuf, &resourceParams));
        if (sfdLineBufferCtrl.Gen12_7.Index != 0)
        {
            cmd.DW28.SfdLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = sfdLineBufferCtrl.Gen12_7.Index;
        }
    }

    // Block 5 (DW38-40): AVS line tile buffer
    if (params.resAvsLineTileBuffer && !Mos_ResourceIsNull(params.resAvsLineTileBuffer))
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource    = params.resAvsLineTileBuffer;
        resourceParams.pdwCmd          = &(cmd.DW38.Value);
        resourceParams.dwLocationInCmd = 38;
        resourceParams.HwCommandType   = MOS_SFC_STATE;
        resourceParams.bIsWritable     = true;
        initMocsFn(resourceParams, &cmd.DW40.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(addResourceFn(pOsInterface, pCmdBuf, &resourceParams));
        if (avsLineTileBufferCtrl.Gen12_7.Index != 0)
        {
            cmd.DW40.AvsLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = avsLineTileBufferCtrl.Gen12_7.Index;
        }
    }

    // Block 6 (DW41-43): IEF line tile buffer
    if (params.resIefLineTileBuffer && !Mos_ResourceIsNull(params.resIefLineTileBuffer))
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource    = params.resIefLineTileBuffer;
        resourceParams.pdwCmd          = &(cmd.DW41.Value);
        resourceParams.dwLocationInCmd = 41;
        resourceParams.HwCommandType   = MOS_SFC_STATE;
        resourceParams.bIsWritable     = true;
        initMocsFn(resourceParams, &cmd.DW43.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(addResourceFn(pOsInterface, pCmdBuf, &resourceParams));
        if (iefLineTileBufferCtrl.Gen12_7.Index != 0)
        {
            cmd.DW43.IefLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = iefLineTileBufferCtrl.Gen12_7.Index;
        }
    }

    // Block 7 (DW44-46): SFD line tile buffer
    if (params.resSfdLineTileBuffer && !Mos_ResourceIsNull(params.resSfdLineTileBuffer))
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource    = params.resSfdLineTileBuffer;
        resourceParams.pdwCmd          = &(cmd.DW44.Value);
        resourceParams.dwLocationInCmd = 44;
        resourceParams.HwCommandType   = MOS_SFC_STATE;
        resourceParams.bIsWritable     = true;
        initMocsFn(resourceParams, &cmd.DW46.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(addResourceFn(pOsInterface, pCmdBuf, &resourceParams));
        if (sfdLineTileBufferCtrl.Gen12_7.Index != 0)
        {
            cmd.DW46.SfdLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = sfdLineTileBufferCtrl.Gen12_7.Index;
        }
    }

    // Block 8 (DW47-49): Histogram surface
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
        initMocsFn(resourceParams, &cmd.DW49.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(addResourceFn(pOsInterface, pCmdBuf, &resourceParams));
        if (histogramBufferCtrl.Gen12_7.Index != 0)
        {
            cmd.DW49.HisgotramBaseAddressIndexToMemoryObjectControlStateMocsTables = histogramBufferCtrl.Gen12_7.Index;
        }
    }

    // Block 9 (DW55-60): Interleaved to field - bottom field address
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
        initMocsFn(resourceParams, &cmd.DW57.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(addResourceFn(pOsInterface, pCmdBuf, &resourceParams));
        if (outputSurfCtrl.Gen12_7.Index != 0)
        {
            cmd.DW57.BottomFieldSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables =
                outputSurfCtrl.Gen12_7.Index;
        }

        cmd.DW58.BottomFieldSurfaceHalfPitchForChroma     = cmd.DW29.OutputSurfaceHalfPitchForChroma;
        cmd.DW58.BottomFieldSurfaceInterleaveChromaEnable = cmd.DW29.OutputSurfaceHalfPitchForChroma;
        cmd.DW58.BottomFieldSurfacePitch                  = cmd.DW29.OutputSurfacePitch;
        cmd.DW58.BottomFieldSurfaceTiled                  = (params.pOutSurface->TileType != MOS_TILE_LINEAR) ? true : false;
        cmd.DW58.BottomFieldSurfaceTileWalk               = (params.pOutSurface->TileType == MOS_TILE_Y) ? true : false;
        cmd.DW59.BottomFieldSurfaceXOffsetForU            = cmd.DW30.OutputSurfaceXOffsetForU;
        cmd.DW59.BottomFieldSurfaceYOffsetForU            = cmd.DW30.OutputSurfaceYOffsetForU;
        cmd.DW60.BottomFieldSurfaceXOffsetForV            = cmd.DW31.OutputSurfaceXOffsetForV;
        cmd.DW60.BottomFieldSurfaceYOffsetForV            = cmd.DW31.OutputSurfaceYOffsetForV;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  SetSfcStateDithering
//!         Sets DW3.DitherEnable and DW50-DW53 LUT deltas.
//!
template <typename cmd_t>
inline void SetSfcStateDithering(
    typename cmd_t::SFC_STATE_CMD &cmd,
    const SFC_STATE_PAR    &params)
{
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
}

//!
//! \brief  SetSfcStateScalability
//!         Handles the entire m_sfcScalabilityEnabled block with parameterized
//!         MaxPipeNum and SfcIndex constants.
//!         maxPipeNum: maximum number of SFC pipes
//!         sfcIndex0/1/2: platform-specific SFC index constants
//!
template <typename cmd_t>
inline void SetSfcStateScalability(
    typename cmd_t::SFC_STATE_CMD &cmd,
    const SFC_STATE_PAR    &params,
    bool                           sfcScalabilityEnabled,
    uint32_t                       numofSfc,
    uint32_t                       indexofSfc,
    uint32_t                       maxPipeNum,
    uint32_t                       sfcIndex0,
    uint32_t                       sfcIndex1,
    uint32_t                       sfcIndex2)
{
    if (!sfcScalabilityEnabled)
    {
        return;
    }

    // Use a fixed upper bound for array sizes to avoid VLAs
    const uint32_t MAX_PIPE_NUM_UPPER_BOUND = 4;
    uint32_t       iMediumX;
    uint32_t       xOffset;
    uint32_t       src_startX[MAX_PIPE_NUM_UPPER_BOUND];
    uint32_t       src_endX[MAX_PIPE_NUM_UPPER_BOUND];
    uint32_t       dest_startX[MAX_PIPE_NUM_UPPER_BOUND];
    uint32_t       dest_endX[MAX_PIPE_NUM_UPPER_BOUND];
    uint32_t       tile_endX;
    uint32_t       dest_first_valid_left_tile = 0;
    uint32_t       dest_last_valid_right_tile = numofSfc - 1;
    uint32_t       dest_cntX                  = 0;
    double         xLandingpoint;
    uint32_t       one_by_sf                    = (uint32_t)(((uint64_t)params.dwSourceRegionWidth * 524288L) / params.dwScaledRegionWidth);
    const uint32_t one_by_sf_fraction_precision = 19;
    const uint32_t beta_precision               = 5;
    int32_t        xPhaseShift;
    double         tempDestCntx;
    uint32_t       i;

    MHW_ASSERT(params.dwInputFrameWidth > numofSfc * 64);

    iMediumX = MOS_ALIGN_FLOOR((params.dwInputFrameWidth / numofSfc), 64);
    iMediumX = MOS_CLAMP_MIN_MAX(iMediumX, 64, (params.dwInputFrameWidth - 64));

    src_startX[0] = dest_startX[0] = 0;

    for (i = 0; i < numofSfc; i++)
    {
        if (i == numofSfc - 1)
        {
            src_endX[i] = params.dwInputFrameWidth - 1;
        }
        else
        {
            src_endX[i] = iMediumX * (i + 1) - 1;
            if (params.dwInputFrameWidth != numofSfc * iMediumX)
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

    for (i = 1; i < numofSfc; i++)
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
    if (numofSfc == 2)
    {
        if (indexofSfc == sfcIndex0)
        {
            cmd.DW1.SfcEngineMode = 1;
        }
        else
        {
            cmd.DW1.SfcEngineMode = 2;
        }
    }
    else if (numofSfc == 3)
    {
        if (indexofSfc == sfcIndex0)
        {
            cmd.DW1.SfcEngineMode = 1;
        }
        else if (indexofSfc == sfcIndex1)
        {
            cmd.DW1.SfcEngineMode = 3;
        }
        else
        {
            cmd.DW1.SfcEngineMode = 2;
        }
    }
    else if (numofSfc == 4)
    {
        if (indexofSfc == sfcIndex0)
        {
            cmd.DW1.SfcEngineMode = 1;
        }
        else if (indexofSfc == sfcIndex1 ||
                 indexofSfc == sfcIndex2)
        {
            cmd.DW1.SfcEngineMode = 3;
        }
        else
        {
            cmd.DW1.SfcEngineMode = 2;
        }
    }

    if (indexofSfc < dest_first_valid_left_tile)
    {
        cmd.DW4.ColorFillEnable = 0;
    }
    else if (indexofSfc > dest_last_valid_right_tile)
    {
        cmd.DW4.ColorFillEnable = 0;
    }

    if (params.bIEFEnable)
    {
        if (dest_startX[indexofSfc] >= 4)
        {
            dest_startX[indexofSfc] -= 4;
        }
        else
        {
            dest_startX[indexofSfc] = 0;
        }
    }

    if (VpHalDDIUtils::GetSurfaceColorPack(params.OutputFrameFormat) != VPHAL_COLORPACK_444 &&
        (dest_startX[indexofSfc] % 2 != 0))
    {
        if (dest_startX[indexofSfc] >= 1)
        {
            dest_startX[indexofSfc] -= 1;
        }
        else
        {
            dest_startX[indexofSfc] = 0;
        }
    }

    cmd.DW34.Sourcestartx      = src_startX[indexofSfc];
    cmd.DW34.Sourceendx        = src_endX[indexofSfc];
    cmd.DW35.Destinationstartx = dest_startX[indexofSfc];
    cmd.DW35.Destinationendx   = dest_endX[indexofSfc];

    MHW_NORMALMESSAGE("SFC%d STATE: src startx %d endx %d", indexofSfc, cmd.DW34.Sourcestartx, cmd.DW34.Sourceendx);
    MHW_NORMALMESSAGE("SFC%d STATE: dest startx %d endx %d", indexofSfc, cmd.DW35.Destinationstartx, cmd.DW35.Destinationendx);
}

//!
//! \brief  SetSfcStateRectangle
//!         Handles the bRectangleEnabled DW61-DW62 block.
//!         Used by Group B and Group C platforms.
//!
template <typename cmd_t>
inline void SetSfcStateRectangle(
    typename cmd_t::SFC_STATE_CMD &cmd,
    const SFC_STATE_PAR    &params)
{
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
        cmd.DW62.TargetRectangleEndHorizontalOffset   = cmd.DW7.OutputFrameWidth;   // target rectangle end offsets are considered as zero based for hw
        cmd.DW62.TargetRectangleEndVerticalOffset     = cmd.DW7.OutputFrameHeight;  // target rectangle end offsets are considered as zero based for hw
    }
}

}  // namespace common
}  // namespace sfc
}  // namespace mhw

#endif  // __MHW_SFC_COMMON_IMPL_H__