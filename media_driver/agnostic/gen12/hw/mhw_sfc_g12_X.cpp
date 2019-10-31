/*
* Copyright (c) 2015-2019, Intel Corporation
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
//! \file     mhw_sfc_g12_X.cpp
//! \brief    Constructs sfc commands on Gen12-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_sfc_g12_X.h"

MOS_STATUS MhwSfcInterfaceG12::AddSfcLock(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    PMHW_SFC_LOCK_PARAMS           pSfcLockParams)
{
    MHW_CHK_NULL_RETURN(pCmdBuffer);
    MHW_CHK_NULL_RETURN(pSfcLockParams);

    mhw_sfc_g12_X::SFC_LOCK_CMD cmd, *cmdPtr;
    cmdPtr = (mhw_sfc_g12_X::SFC_LOCK_CMD *)pCmdBuffer->pCmdPtr;

    MHW_CHK_STATUS_RETURN(MhwSfcInterfaceGeneric::AddSfcLock(pCmdBuffer, pSfcLockParams));

    if (pSfcLockParams->sfcPipeMode == MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP)
    {
        MHW_CHK_NULL_RETURN(cmdPtr);
        cmdPtr->DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwSfcInterfaceG12::AddSfcState(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    PMHW_SFC_STATE_PARAMS          pSfcStateParams,
    PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface)
{
    PMOS_INTERFACE              pOsInterface;
    bool                        bHalfPitchForChroma;
    bool                        bInterleaveChroma;
    uint16_t                    wUXOffset;
    uint16_t                    wUYOffset;
    uint16_t                    wVXOffset;
    uint16_t                    wVYOffset;
    MHW_RESOURCE_PARAMS         ResourceParams;
    MEDIA_WA_TABLE              *pWaTable = nullptr;

    PMHW_SFC_STATE_PARAMS_G12       pSfcStateParamsG12 = static_cast<PMHW_SFC_STATE_PARAMS_G12>(pSfcStateParams);
    mhw_sfc_g12_X::SFC_STATE_CMD    cmd;

    MHW_CHK_NULL_RETURN(pCmdBuffer);
    MHW_CHK_NULL_RETURN(pSfcStateParams);
    MHW_CHK_NULL_RETURN(pOutSurface);

    pOsInterface    = m_osInterface;
    MHW_CHK_NULL_RETURN(pOsInterface);
    pWaTable        = pOsInterface->pfnGetWaTable(pOsInterface);
    MHW_CHK_NULL_RETURN(pWaTable);

    bHalfPitchForChroma = false;
    bInterleaveChroma   = false;
    wUXOffset           = 0;
    wUYOffset           = 0;
    wVXOffset           = 0;
    wVYOffset           = 0;

    // Check input/output size
    MHW_ASSERT(pSfcStateParamsG12->dwInputFrameWidth   >= MHW_SFC_MIN_WIDTH);
    MHW_ASSERT(pSfcStateParamsG12->dwInputFrameHeight  >= MHW_SFC_MIN_HEIGHT);
    MHW_ASSERT(pSfcStateParamsG12->dwOutputFrameWidth  <= MHW_SFC_MAX_WIDTH);
    MHW_ASSERT(pSfcStateParamsG12->dwOutputFrameHeight <= MHW_SFC_MAX_HEIGHT);

    // Set DW0
    if (pSfcStateParamsG12->sfcPipeMode == MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP)
    {
        cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHCPSFCMODE;
    }
    else
    {
        cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
    }

    // Set DW1
    cmd.DW1.SfcPipeMode                  = pSfcStateParamsG12->sfcPipeMode;
    cmd.DW1.SfcInputChromaSubSampling    = pSfcStateParamsG12->dwInputChromaSubSampling;
    cmd.DW1.VdVeInputOrderingMode        = pSfcStateParamsG12->dwVDVEInputOrderingMode;
    cmd.DW1.SfcEngineMode                = pSfcStateParamsG12->engineMode;
    cmd.DW1.SfcInputStreamBitDepth       = pSfcStateParamsG12->inputBitDepth;

    // Set DW2
    cmd.DW2.InputFrameResolutionWidth    = pSfcStateParamsG12->dwInputFrameWidth - 1;
    cmd.DW2.InputFrameResolutionHeight   = pSfcStateParamsG12->dwInputFrameHeight - 1;

    // Set DW3
    cmd.DW3.OutputChromaDownsamplingCoSitingPositionVerticalDirection      = pSfcStateParamsG12->dwChromaDownSamplingVerticalCoef;
    cmd.DW3.OutputChromaDownsamplingCoSitingPositionHorizontalDirection    = pSfcStateParamsG12->dwChromaDownSamplingHorizontalCoef;
    cmd.DW3.InputColorSpace0Yuv1Rgb                                        = pSfcStateParamsG12->bInputColorSpace;

    switch(pSfcStateParamsG12->OutputFrameFormat)
    {
        case Format_AYUV:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_AYUV;
            break;
        case Format_X8R8G8B8:
        case Format_A8R8G8B8:
        case Format_X8B8G8R8:
        case Format_A8B8G8R8:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R8;
            break;
        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_A2R10G10B10;
            break;
        case Format_R5G6B5:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_R5G6B5;
            break;
        case Format_NV12:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_NV12;
            bInterleaveChroma   = true;
            wUYOffset           = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_YUY2:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_YUYV;
            break;
        case Format_UYVY:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_UYVY;
            break;
        case Format_P010:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_P016;
            cmd.DW4.Bitdepth                = 0;
            wUYOffset                       = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_P016:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_P016;
            cmd.DW4.Bitdepth                = 1;
            wUYOffset                       = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_Y210:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_Y216;
            cmd.DW4.Bitdepth                = 0;
            wUYOffset                       = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_Y216:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_Y216;
            cmd.DW4.Bitdepth                = 1;
            wUYOffset                       = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_Y410:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_Y416;
            cmd.DW4.Bitdepth                = 0;
            wUYOffset                       = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_Y416:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_Y416;
            cmd.DW4.Bitdepth                = 1;
            wUYOffset                       = (uint16_t)pOutSurface->dwUYoffset;
            break;
        default:
            MHW_ASSERTMESSAGE("Unknown Output Format.");
            return MOS_STATUS_UNKNOWN;

    }

    // RGBASwapEnable is true when the OutputSurfaceFormatType is set as A8B8G8R8 for X8R8G8B8 and A8R8G8B8 output,
    // and the OutputSurfaceFormatType is set as A2R10G10B10 for R10G10B10A2 output.
    cmd.DW3.RgbaChannelSwapEnable = pSfcStateParamsG12->bRGBASwapEnable;

    // Set DW4
    cmd.DW4.IefEnable                    = pSfcStateParamsG12->bIEFEnable;
    cmd.DW4.SkinToneTunedIefEnable       = pSfcStateParamsG12->bSkinToneTunedIEFEnable;
    cmd.DW4.AvsFilterMode                = pSfcStateParamsG12->dwAVSFilterMode;
    if (pSfcStateParamsG12->b8tapChromafiltering)
    {
        cmd.DW4.AdaptiveFilterForAllChannels = true;
    }
    else
    {
        cmd.DW4.AdaptiveFilterForAllChannels = false;
    }

    cmd.DW4.AvsScalingEnable                     = ((pSfcStateParamsG12->fAVSXScalingRatio == 1.0F) &&
                                                             (pSfcStateParamsG12->fAVSYScalingRatio == 1.0F)) ? false : true;
    cmd.DW4.BypassYAdaptiveFiltering             = pSfcStateParamsG12->bBypassYAdaptiveFilter;
    cmd.DW4.BypassXAdaptiveFiltering             = pSfcStateParamsG12->bBypassXAdaptiveFilter;
    cmd.DW4.ChromaUpsamplingEnable               = pSfcStateParamsG12->bAVSChromaUpsamplingEnable;
    cmd.DW4.RotationMode                         = pSfcStateParamsG12->RotationMode;
    cmd.DW4.ColorFillEnable                      = pSfcStateParamsG12->bColorFillEnable;
    cmd.DW4.CscEnable                            = pSfcStateParamsG12->bCSCEnable;
    cmd.DW4.Enable8TapForChromaChannelsFiltering = pSfcStateParamsG12->b8tapChromafiltering;
    cmd.DW4.TileType                             = pSfcStateParamsG12->tileType;
    cmd.DW4.RgbAdaptive                          = pSfcStateParamsG12->bRGBAdaptive;

    if (pSfcStateParams->bMirrorEnable)
    {
        cmd.DW4.Value |= (uint32_t)(1 << 14) & 0x00004000;                                          // Mirror Mode
        cmd.DW4.Value |= (uint32_t)(pSfcStateParams->dwMirrorType << 13) & 0x00002000;              // Mirror Type
    }

    // Set DW5, DW6, DW7, DW8, DW9
    cmd.DW5.SourceRegionWidth            = pSfcStateParamsG12->dwSourceRegionWidth - 1;
    cmd.DW5.SourceRegionHeight           = pSfcStateParamsG12->dwSourceRegionHeight - 1;
    cmd.DW6.SourceRegionHorizontalOffset = pSfcStateParamsG12->dwSourceRegionHorizontalOffset;
    cmd.DW6.SourceRegionVerticalOffset   = pSfcStateParamsG12->dwSourceRegionVerticalOffset;
    cmd.DW7.OutputFrameWidth             = pSfcStateParamsG12->dwOutputFrameWidth + pOutSurface->dwSurfaceXOffset - 1;
    cmd.DW7.OutputFrameHeight            = pSfcStateParamsG12->dwOutputFrameHeight + pOutSurface->dwSurfaceYOffset - 1;
    cmd.DW8.ScaledRegionSizeWidth        = pSfcStateParamsG12->dwScaledRegionWidth - 1;
    cmd.DW8.ScaledRegionSizeHeight       = pSfcStateParamsG12->dwScaledRegionHeight - 1;
    cmd.DW9.ScaledRegionHorizontalOffset = pSfcStateParamsG12->dwScaledRegionHorizontalOffset + pOutSurface->dwSurfaceXOffset;
    cmd.DW9.ScaledRegionVerticalOffset   = pSfcStateParamsG12->dwScaledRegionVerticalOffset + pOutSurface->dwSurfaceYOffset;

    // Set DW10
    cmd.DW10.GrayBarPixelUG              = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParamsG12->fColorFillUGPixel * 1024.0F), 0, 1023); // U10
    cmd.DW10.GrayBarPixelYR              = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParamsG12->fColorFillYRPixel * 1024.0F), 0, 1023); // U10

    // Set DW11
    cmd.DW11.GrayBarPixelA               = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParamsG12->fColorFillAPixel * 1024.0F), 0, 1023); // U10
    cmd.DW11.GrayBarPixelVB              = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParamsG12->fColorFillVBPixel * 1024.0F), 0, 1023); // U10

    // Set DW13
    cmd.DW13.AlphaDefaultValue           = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParamsG12->fAlphaPixel * 1024.0F), 0, 1023); // U10

    // Set DW14
    cmd.DW14.ScalingFactorHeight         = MOS_UF_ROUND((double)pSfcStateParamsG12->dwSourceRegionHeight / (double)pSfcStateParamsG12->dwScaledRegionHeight * 524288.0F); // U4.19

    // Set DW15
    cmd.DW15.ScalingFactorWidth          = MOS_UF_ROUND((double)pSfcStateParamsG12->dwSourceRegionWidth / (double)pSfcStateParamsG12->dwScaledRegionWidth * 524288.0F); // U4.19

    // Set DW19
    if (pSfcStateParamsG12->MMCMode == MOS_MMC_RC || pSfcStateParamsG12->MMCMode == MOS_MMC_MC)
    {
        cmd.DW19.OutputFrameSurfaceBaseAddressMemoryCompressionEnable = pSfcStateParamsG12->bMMCEnable;
    }    
    cmd.DW19.OutputFrameSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables = m_outputSurfCtrl.Gen9.Index;

    cmd.DW19.OutputFrameSurfaceBaseAddressMemoryCompressionMode = (pSfcStateParamsG12->MMCMode == MOS_MMC_RC) ? 1 : 0;

    // Set DW22
    cmd.DW22.AvsLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables
                                                   = m_avsLineBufferCtrl.Gen9.Index;
    // Set DW25
    cmd.DW25.IefLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables
                                                   = m_iefLineBufferCtrl.Gen9.Index;

    // Set DW29
    cmd.DW29.OutputSurfaceTileWalk       = (pOutSurface->TileType == MOS_TILE_Y) ? 
                                                      true : false;
    cmd.DW29.OutputSurfaceTiled          = (pOutSurface->TileType != MOS_TILE_LINEAR) ?
                                                      true : false;
    cmd.DW29.OutputSurfaceHalfPitchForChroma 
                                                   = bHalfPitchForChroma;
    cmd.DW29.OutputSurfacePitch          = pOutSurface->dwPitch - 1;
    cmd.DW29.OutputSurfaceInterleaveChromaEnable 
                                                   = bInterleaveChroma;
    cmd.DW29.OutputSurfaceFormat         = cmd.DW3.OutputSurfaceFormatType;

    // Set DW30, DW31
    cmd.DW30.OutputSurfaceYOffsetForU    = wUYOffset;
    cmd.DW30.OutputSurfaceXOffsetForU    = wUXOffset;
    cmd.DW31.OutputSurfaceYOffsetForV    = wVYOffset;
    cmd.DW31.OutputSurfaceXOffsetForV    = wVXOffset;

    // DW34, DW35
    cmd.DW34.SourceStartX         = pSfcStateParamsG12->srcStartX;
    cmd.DW34.SourceEndX           = pSfcStateParamsG12->srcEndX;
    cmd.DW35.DestinationStartX    = pSfcStateParamsG12->dstStartX;
    cmd.DW35.DestinationEndX      = pSfcStateParamsG12->dstEndX;

    // Set DW36, DW37
    //Change SFC outputcentering scaling X/Yphaseshift value and limition limitione with 19bit following Fuslim setting.
    if (m_outputCenteringEnable)
    {
        cmd.DW36.Xphaseshift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)cmd.DW15.ScalingFactorWidth / 524288.0F - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
        cmd.DW37.Yphaseshift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)cmd.DW14.ScalingFactorHeight / 524288.0F - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
    }

    if (pSfcStateParamsG12->pOsResOutputSurface)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource                = pSfcStateParamsG12->pOsResOutputSurface;
        ResourceParams.pdwCmd                      = &(cmd.DW17.Value);
        ResourceParams.dwLocationInCmd             = 17;
        ResourceParams.HwCommandType               = MOS_SFC_STATE;
        ResourceParams.bIsWritable                 = true;
        ResourceParams.dwOffset                    = pSfcStateParamsG12->dwOutputSurfaceOffset;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
          pOsInterface,
          pCmdBuffer,
          &ResourceParams));
    }

    if (pSfcStateParamsG12->pOsResAVSLineBuffer)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource                = pSfcStateParamsG12->pOsResAVSLineBuffer;
        ResourceParams.pdwCmd                      = &(cmd.DW20.Value);
        ResourceParams.dwLocationInCmd             = 20;
        ResourceParams.HwCommandType               = MOS_SFC_STATE;
        ResourceParams.bIsWritable                 = true;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
          pOsInterface,
          pCmdBuffer,
          &ResourceParams));
    }

    if (pSfcStateParamsG12->pOsResIEFLineBuffer)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource                = pSfcStateParamsG12->pOsResIEFLineBuffer;
        ResourceParams.pdwCmd                      = &(cmd.DW23.Value);
        ResourceParams.dwLocationInCmd             = 23;
        ResourceParams.HwCommandType               = MOS_SFC_STATE;
        ResourceParams.bIsWritable                 = true;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
          pOsInterface,
          pCmdBuffer,
          &ResourceParams));
    }

    if (pSfcStateParamsG12->resSfdLineBuffer && !Mos_ResourceIsNull(pSfcStateParamsG12->resSfdLineBuffer))
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource = pSfcStateParamsG12->resSfdLineBuffer;
        ResourceParams.pdwCmd = &(cmd.DW26.Value);
        ResourceParams.dwLocationInCmd = 26;
        ResourceParams.HwCommandType = MOS_SFC_STATE;
        ResourceParams.bIsWritable = true;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pSfcStateParamsG12->resAvsLineTileBuffer && !Mos_ResourceIsNull(pSfcStateParamsG12->resAvsLineTileBuffer))
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource = pSfcStateParamsG12->resAvsLineTileBuffer;
        ResourceParams.pdwCmd = &(cmd.DW38.Value);
        ResourceParams.dwLocationInCmd = 38;
        ResourceParams.HwCommandType = MOS_SFC_STATE;
        ResourceParams.bIsWritable = true;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pSfcStateParamsG12->resSfdLineTileBuffer && !Mos_ResourceIsNull(pSfcStateParamsG12->resSfdLineTileBuffer))
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource = pSfcStateParamsG12->resSfdLineTileBuffer;
        ResourceParams.pdwCmd = &(cmd.DW44.Value);
        ResourceParams.dwLocationInCmd = 44;
        ResourceParams.HwCommandType = MOS_SFC_STATE;
        ResourceParams.bIsWritable = true;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pSfcStateParamsG12->histogramSurface && !Mos_ResourceIsNull(&pSfcStateParamsG12->histogramSurface->OsResource))
    {
        cmd.DW4.HistogramStreamout                 = 1;
        cmd.DW49.Value                             = 0;

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource                = &pSfcStateParamsG12->histogramSurface->OsResource;
        ResourceParams.dwOffset                    = pSfcStateParamsG12->histogramSurface->dwOffset;
        ResourceParams.pdwCmd                      = &(cmd.DW47.Value);
        ResourceParams.dwLocationInCmd             = 47;
        ResourceParams.HwCommandType               = MOS_SFC_STATE;
        ResourceParams.bIsWritable                 = true;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    MHW_CHK_STATUS_RETURN(Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwSfcInterfaceG12::AddSfcAvsState(
    PMOS_COMMAND_BUFFER             pCmdBuffer,
    PMHW_SFC_AVS_STATE              pSfcAvsState)
{
    MHW_CHK_NULL_RETURN(pCmdBuffer);
    MHW_CHK_NULL_RETURN(pSfcAvsState);
    mhw_sfc_g12_X::SFC_AVS_STATE_CMD cmd, *cmdPtr;
    cmdPtr = (mhw_sfc_g12_X::SFC_AVS_STATE_CMD *)pCmdBuffer->pCmdPtr;

    MHW_CHK_STATUS_RETURN(MhwSfcInterfaceGeneric::AddSfcAvsState(pCmdBuffer, pSfcAvsState));

    MHW_CHK_NULL_RETURN(cmdPtr);
    if (pSfcAvsState->sfcPipeMode == MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP)
    {
        cmdPtr->DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
    }
    else
    {
        cmdPtr->DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
    }

    cmdPtr->DW3.InputHorizontalSitingValueSpecifiesTheHorizontalSitingOfTheInput = pSfcAvsState->dwInputHorizontalSiting;
    cmdPtr->DW3.InputVerticalSitingSpecifiesTheVerticalSitingOfTheInput          = pSfcAvsState->dwInputVerticalSitting;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwSfcInterfaceG12::AddSfcFrameStart(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    uint8_t                        sfcPipeMode)
{
    MHW_CHK_NULL_RETURN(pCmdBuffer);
    mhw_sfc_g12_X::SFC_FRAME_START_CMD cmd, *cmdPtr;
    cmdPtr = (mhw_sfc_g12_X::SFC_FRAME_START_CMD *)pCmdBuffer->pCmdPtr;

    MHW_CHK_STATUS_RETURN(MhwSfcInterfaceGeneric::AddSfcFrameStart(pCmdBuffer, sfcPipeMode));
    MHW_CHK_NULL_RETURN(cmdPtr);
    if (sfcPipeMode == MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP)
    {
        cmdPtr->DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwSfcInterfaceG12::AddSfcIefState(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    PMHW_SFC_IEF_STATE_PARAMS      pSfcIefStateParams)
{
    MHW_CHK_NULL_RETURN(pCmdBuffer);
    MHW_CHK_NULL_RETURN(pSfcIefStateParams);

    mhw_sfc_g12_X::SFC_IEF_STATE_CMD cmd, *cmdPtr;
    cmdPtr = (mhw_sfc_g12_X::SFC_IEF_STATE_CMD *)pCmdBuffer->pCmdPtr;

    MHW_CHK_STATUS_RETURN(MhwSfcInterfaceGeneric::AddSfcIefState(pCmdBuffer, pSfcIefStateParams));

    if (pSfcIefStateParams->sfcPipeMode == MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP)
    {
        MHW_CHK_NULL_RETURN(cmdPtr);
        cmdPtr->DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwSfcInterfaceG12::AddSfcAvsChromaTable(
    PMOS_COMMAND_BUFFER             pCmdBuffer,
    PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable)
{
    MHW_CHK_NULL_RETURN(pCmdBuffer);
    MHW_CHK_NULL_RETURN(pChromaTable);

    mhw_sfc_g12_X::SFC_AVS_CHROMA_Coeff_Table_CMD cmd, *cmdPtr;
    cmdPtr = (mhw_sfc_g12_X::SFC_AVS_CHROMA_Coeff_Table_CMD *)pCmdBuffer->pCmdPtr;
    MHW_CHK_STATUS_RETURN(MhwSfcInterfaceGeneric::AddSfcAvsChromaTable(pCmdBuffer, pChromaTable));

    if (pChromaTable->sfcPipeMode == MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP) // HCP-to-SFC
    {
        MHW_CHK_NULL_RETURN(cmdPtr);
        cmdPtr->DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwSfcInterfaceG12::AddSfcAvsLumaTable(
    PMOS_COMMAND_BUFFER             pCmdBuffer,
    PMHW_SFC_AVS_LUMA_TABLE         pLumaTable)
{
    MHW_CHK_NULL_RETURN(pCmdBuffer);
    MHW_CHK_NULL_RETURN(pLumaTable);

    mhw_sfc_g12_X::SFC_AVS_LUMA_Coeff_Table_CMD cmd, *cmdPtr;
    cmdPtr = (mhw_sfc_g12_X::SFC_AVS_LUMA_Coeff_Table_CMD *)pCmdBuffer->pCmdPtr;
    MHW_CHK_STATUS_RETURN(MhwSfcInterfaceGeneric::AddSfcAvsLumaTable(pCmdBuffer, pLumaTable));

    if (pLumaTable->sfcPipeMode == MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP) // HCP-to-SFC
    {
        MHW_CHK_NULL_RETURN(cmdPtr);
        cmdPtr->DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwSfcInterfaceG12 :: SetSfcSamplerTable(
    PMHW_SFC_AVS_LUMA_TABLE         pLumaTable,
    PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable,
    PMHW_AVS_PARAMS                 pAvsParams,
    MOS_FORMAT                      SrcFormat,
    float                           fScaleX,
    float                           fScaleY,
    uint32_t                        dwChromaSiting,
    bool                            bUse8x8Filter)
{
    float       fHPStrength;
    int32_t     *piYCoefsX,  *piYCoefsY;
    int32_t     *piUVCoefsX, *piUVCoefsY;
    MHW_PLANE   Plane;

    MHW_CHK_NULL_RETURN(pLumaTable);
    MHW_CHK_NULL_RETURN(pChromaTable);
    MHW_CHK_NULL_RETURN(pAvsParams);

    fHPStrength = 0.0F;
    piYCoefsX   = pAvsParams->piYCoefsX;
    piYCoefsY   = pAvsParams->piYCoefsY;
    piUVCoefsX  = pAvsParams->piUVCoefsX;
    piUVCoefsY  = pAvsParams->piUVCoefsY;

    // Skip calculation if no changes to AVS parameters
    if (SrcFormat == pAvsParams->Format  &&
        fScaleX   == pAvsParams->fScaleX &&
        fScaleY   == pAvsParams->fScaleY)
    {
        return MOS_STATUS_SUCCESS;
    }

    // AVS Coefficients don't change for Scaling Factors > 1.0x
    // Hence recalculation is avoided
    if (fScaleX > 1.0F && pAvsParams->fScaleX > 1.0F)
    {
        pAvsParams->fScaleX = fScaleX;
    }

    // AVS Coefficients don't change for Scaling Factors > 1.0x
    // Hence recalculation is avoided
    if (fScaleY > 1.0F && pAvsParams->fScaleY > 1.0F)
    {
        pAvsParams->fScaleY = fScaleY;
    }

    // Recalculate Horizontal scaling table
    if (SrcFormat != pAvsParams->Format || fScaleX != pAvsParams->fScaleX)
    {
        MOS_ZeroMemory(
            piYCoefsX, 
            8 * 32 * sizeof(int32_t));

        MOS_ZeroMemory(
            piUVCoefsX, 
            4 * 32 * sizeof(int32_t));

        Plane = (IS_RGB32_FORMAT(SrcFormat) && !bUse8x8Filter) ? MHW_U_PLANE : MHW_Y_PLANE;

        pAvsParams->fScaleX = fScaleX;

        // For 1x scaling in horizontal direction and not force polyphase coefs, use special coefficients for filtering
        if (fScaleX == 1.0F && !pAvsParams->bForcePolyPhaseCoefs)
        {
            MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                piYCoefsX,
                Plane,
                true));

            // The 8-tap adaptive is enabled for all channel if RGB format input, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!(IS_RGB32_FORMAT(SrcFormat) && bUse8x8Filter))
            {
                MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                    piUVCoefsX,
                    MHW_U_PLANE,
                    true));
            }
        }
        else
        {
            // Clamp the Scaling Factor if > 1.0x
            fScaleX = MOS_MIN(1.0F, fScaleX);

            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                piYCoefsX,
                fScaleX,
                Plane,
                SrcFormat,
                fHPStrength,
                bUse8x8Filter,
                NUM_HW_POLYPHASE_TABLES));
        }

        // The 8-tap adaptive is enabled for all channel if RGB format input, then UV/RB use the same coefficient as Y/G
        // So, coefficient for UV/RB channels caculation can be passed
        if (!(IS_RGB32_FORMAT(SrcFormat) && bUse8x8Filter))
        {
            // If Chroma Siting info is present
            if (dwChromaSiting & MHW_CHROMA_SITING_HORZ_LEFT)
            {
                // No Chroma Siting
                MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                    piUVCoefsX,
                    2.0F,
                    fScaleX));
            }
            else
            {
                // Chroma siting offset will be add in the HW cmd
                MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                    piUVCoefsX,
                    3.0F,
                    fScaleX));
            }
        }
    }

    // Recalculate Vertical scaling table
    if (SrcFormat != pAvsParams->Format || fScaleY != pAvsParams->fScaleY)
    {
        memset((void *)piYCoefsY, 0, 8 * 32 * sizeof(int32_t));

        memset((void *)piUVCoefsY, 0, 4 * 32 * sizeof(int32_t));

        Plane = (IS_RGB32_FORMAT(SrcFormat) && !bUse8x8Filter) ? MHW_U_PLANE : MHW_Y_PLANE;

        pAvsParams->fScaleY = fScaleY;

        // For 1x scaling in vertical direction and not force polyphase coefs, use special coefficients for filtering
        if (fScaleY == 1.0F && !pAvsParams->bForcePolyPhaseCoefs)
        {
            MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                piYCoefsY,
                Plane,
                true));

            // The 8-tap adaptive is enabled for all channel if RGB format input, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!(IS_RGB32_FORMAT(SrcFormat) && bUse8x8Filter))
            {
                MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                    piUVCoefsY,
                    MHW_U_PLANE,
                    true));
            }
        }
        else
        {
            // Clamp the Scaling Factor if > 1.0x
            fScaleY = MOS_MIN(1.0F, fScaleY);

            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                piYCoefsY,
                fScaleY,
                Plane,
                SrcFormat,
                fHPStrength,
                bUse8x8Filter,
                NUM_HW_POLYPHASE_TABLES));
        }

        // The 8-tap adaptive is enabled for all channel if RGB format input, then UV/RB use the same coefficient as Y/G
        // So, coefficient for UV/RB channels caculation can be passed
        if (!(IS_RGB32_FORMAT(SrcFormat) && bUse8x8Filter))
        {
            // If Chroma Siting info is present
            if (dwChromaSiting & MHW_CHROMA_SITING_VERT_TOP)
            {
                // No Chroma Siting
                MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                    piUVCoefsY,
                    2.0F,
                    fScaleY));
            }
            else
            {
                // Chroma siting offset will be add in the HW cmd
                MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                    piUVCoefsY,
                    3.0F,
                    fScaleY));
            }
        }
    }

    // Save format used to calculate AVS parameters
    pAvsParams->Format = SrcFormat;

    MhwSfcInterface::SetSfcAVSLumaTable(
        SrcFormat,
        pLumaTable->LumaTable,
        piYCoefsX,
        piYCoefsY,
        bUse8x8Filter);

    MhwSfcInterface::SetSfcAVSChromaTable(
        pChromaTable->ChromaTable,
        piUVCoefsX,
        piUVCoefsY);

    return MOS_STATUS_SUCCESS;
}

MhwSfcInterfaceG12::MhwSfcInterfaceG12(PMOS_INTERFACE pOsInterface) : MhwSfcInterfaceGeneric(pOsInterface)
{
    if (m_osInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid Input Parameter: m_osInterface is nullptr");
        return;
    }

    // Get Memory control object directly from MOS.
    // If any override is needed, something like pfnOverrideMemoryObjectCtrl() / pfnComposeSurfaceCacheabilityControl()
    // will need to be implemented.
    m_outputSurfCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;

    m_avsLineBufferCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;
    m_iefLineBufferCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;

    m_maxWidth  = MHW_SFC_MAX_WIDTH_G12;
    m_maxHeight = MHW_SFC_MAX_HEIGHT_G12;
}

void MhwSfcInterfaceG12::IsOutPutCenterEnable(bool inputEnable)
{
    m_outputCenteringEnable = inputEnable;
}
