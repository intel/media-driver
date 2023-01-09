/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_sfc_xe_xpm.cpp
//! \brief    Constructs sfc commands on Gen12-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "vphal_common.h"
#include "mhw_sfc_xe_xpm.h"
#include "mhw_sfc_hwcmd_xe_xpm.h"
#include "mhw_utilities_xe_xpm.h"
#include "vp_hal_ddi_utils.h"

#define VALUE_XOFFSET 3

MhwSfcInterfaceXe_Xpm::MhwSfcInterfaceXe_Xpm(PMOS_INTERFACE pOsInterface)
    : MhwSfcInterfaceG12(pOsInterface)
{
    MHW_FUNCTION_ENTER;

    m_sfcScalabilitySupported = false;
    m_sfcScalabilityEnabled   = false;
    m_indexofSfc              = 0;
    m_numofSfc                = 1;

    MHW_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

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
    m_sfdLineBufferCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MP_RESOURCE_USAGE_DEFAULT,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;
    m_avsLineTileBufferCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;
    m_iefLineTileBufferCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;
    m_sfdLineTileBufferCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MP_RESOURCE_USAGE_DEFAULT,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;
    m_histogramBufferCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MP_RESOURCE_USAGE_DEFAULT,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;
}

MhwSfcInterfaceXe_Xpm::~MhwSfcInterfaceXe_Xpm()
{
    MHW_FUNCTION_ENTER;
}

MOS_STATUS MhwSfcInterfaceXe_Xpm::AddSfcState(
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_SFC_STATE_PARAMS       pSfcStateParams,
    PMHW_SFC_OUT_SURFACE_PARAMS pOutSurface)
{
    PMOS_INTERFACE                 pOsInterface;
    bool                           bHalfPitchForChroma;
    bool                           bInterleaveChroma;
    uint16_t                       wUXOffset;
    uint16_t                       wUYOffset;
    uint16_t                       wVXOffset;
    uint16_t                       wVYOffset;
    MHW_RESOURCE_PARAMS            ResourceParams;
    MEDIA_WA_TABLE                 *pWaTable = nullptr;
    PMHW_SFC_STATE_PARAMS_XE_XPM      pSfcStateparamsXe_Xpm;
    mhw_sfc_xe_xpm::SFC_STATE_CMD cmd;

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

    pSfcStateparamsXe_Xpm = static_cast<PMHW_SFC_STATE_PARAMS_XE_XPM>(pSfcStateParams);

    // Check input/output size
    MHW_ASSERT(pSfcStateparamsXe_Xpm->dwInputFrameWidth   >= m_minWidth);
    MHW_ASSERT(pSfcStateparamsXe_Xpm->dwInputFrameHeight  >= m_minHeight);
    MHW_ASSERT(pSfcStateparamsXe_Xpm->dwOutputFrameWidth  <= m_maxWidth);
    MHW_ASSERT(pSfcStateparamsXe_Xpm->dwOutputFrameHeight <= m_maxHeight);

    // Set DW0
    if (pSfcStateparamsXe_Xpm->sfcPipeMode == MhwSfcInterfaceG12::SFC_PIPE_MODE_HCP)
    {
        cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAHCPSFCMODE;
    }
    else
    {
        cmd.DW0.MediaCommandOpcode = cmd.MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
    }

    // Set DW1
    cmd.DW1.SfcPipeMode                  = pSfcStateparamsXe_Xpm->sfcPipeMode;
    cmd.DW1.SfcInputChromaSubSampling    = pSfcStateparamsXe_Xpm->dwInputChromaSubSampling;
    cmd.DW1.VdVeInputOrderingMode        = pSfcStateparamsXe_Xpm->dwVDVEInputOrderingMode;
    cmd.DW1.SfcEngineMode                = pSfcStateparamsXe_Xpm->engineMode;

    // Set DW2
    cmd.DW2.InputFrameResolutionWidth    = pSfcStateparamsXe_Xpm->dwInputFrameWidth - 1;
    cmd.DW2.InputFrameResolutionHeight   = pSfcStateparamsXe_Xpm->dwInputFrameHeight - 1;

    // Set DW3
    cmd.DW3.OutputChromaDownsamplingCoSitingPositionVerticalDirection      = pSfcStateparamsXe_Xpm->dwChromaDownSamplingVerticalCoef;
    cmd.DW3.OutputChromaDownsamplingCoSitingPositionHorizontalDirection    = pSfcStateparamsXe_Xpm->dwChromaDownSamplingHorizontalCoef;
    cmd.DW3.InputColorSpace0Yuv1Rgb                                        = pSfcStateparamsXe_Xpm->bInputColorSpace;

    switch(pSfcStateparamsXe_Xpm->OutputFrameFormat)
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
        case Format_YVYU:
        case Format_YUY2:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_YUYV;
            break;
        case Format_VYUY:
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
        case Format_Y8:
            cmd.DW3.OutputSurfaceFormatType = 13;
            cmd.DW4.Bitdepth                = 0;
            break;
        case Format_Y16U:
        case Format_Y16S:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R83;
            cmd.DW4.Bitdepth                = 0;
            break;
        case Format_A16R16G16B16:
        case Format_A16B16G16R16:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R84;
            cmd.DW4.Bitdepth                = 0;
            break;
        default:
            MHW_ASSERTMESSAGE("Unknown Output Format.");
            return MOS_STATUS_UNKNOWN;
    }

    // RGBASwapEnable is true when the OutputSurfaceFormatType is set as A8B8G8R8 for X8R8G8B8 and A8R8G8B8 output,
    // the OutputSurfaceFormatType is set as A2R10G10B10 for R10G10B10A2 output,
    // the OutputSurfaceFormatType is set as YUYV for YVYU output,
    // the OutputSurfaceFormatType is set as UYVY for VYUY output and
    // the OutputSurfaceFormatType is set as A8B8G8R84 for A16R16G16B16 output.
    cmd.DW3.ChannelSwapEnable = pSfcStateparamsXe_Xpm->bRGBASwapEnable;
    // Set DW4
    cmd.DW4.IefEnable                    = pSfcStateparamsXe_Xpm->bIEFEnable;
    cmd.DW4.SkinToneTunedIefEnable       = pSfcStateparamsXe_Xpm->bSkinToneTunedIEFEnable;
    cmd.DW4.AvsFilterMode                = pSfcStateparamsXe_Xpm->dwAVSFilterMode;
    if (pSfcStateparamsXe_Xpm->b8tapChromafiltering)
    {
        cmd.DW4.AdaptiveFilterForAllChannels = true;
    }
    else
    {
        cmd.DW4.AdaptiveFilterForAllChannels = false;
    }

    cmd.DW4.AvsScalingEnable                     = ((pSfcStateparamsXe_Xpm->fAVSXScalingRatio == 1.0F) &&
        (pSfcStateparamsXe_Xpm->fAVSYScalingRatio == 1.0F)) ? false : true;
    cmd.DW4.BypassYAdaptiveFiltering             = pSfcStateparamsXe_Xpm->bBypassYAdaptiveFilter;
    cmd.DW4.BypassXAdaptiveFiltering             = pSfcStateparamsXe_Xpm->bBypassXAdaptiveFilter;
    cmd.DW4.ChromaUpsamplingEnable               = pSfcStateparamsXe_Xpm->bAVSChromaUpsamplingEnable;
    cmd.DW4.RotationMode                         = pSfcStateparamsXe_Xpm->RotationMode;
    cmd.DW4.ColorFillEnable                      = pSfcStateparamsXe_Xpm->bColorFillEnable;
    cmd.DW4.CscEnable                            = pSfcStateparamsXe_Xpm->bCSCEnable;
    cmd.DW4.Enable8TapForChromaChannelsFiltering = pSfcStateparamsXe_Xpm->b8tapChromafiltering;
    cmd.DW4.TileType                             = pSfcStateparamsXe_Xpm->tileType;
    cmd.DW4.RgbAdaptive                          = pSfcStateparamsXe_Xpm->bRGBAdaptive;

    if (pSfcStateParams->bMirrorEnable)
    {
        cmd.DW4.Value |= (uint32_t)(1 << 14) & 0x00004000;                                          // Mirror Mode
        cmd.DW4.Value |= (uint32_t)(pSfcStateParams->dwMirrorType << 13) & 0x00002000;              // Mirror Type
    }

    // Set DW5, DW6, DW7, DW8, DW9
    cmd.DW5.SourceRegionWidth            = pSfcStateparamsXe_Xpm->dwSourceRegionWidth - 1;
    cmd.DW5.SourceRegionHeight           = pSfcStateparamsXe_Xpm->dwSourceRegionHeight - 1;
    cmd.DW6.SourceRegionHorizontalOffset = pSfcStateparamsXe_Xpm->dwSourceRegionHorizontalOffset;
    cmd.DW6.SourceRegionVerticalOffset   = pSfcStateparamsXe_Xpm->dwSourceRegionVerticalOffset;
    cmd.DW7.OutputFrameWidth             = pSfcStateparamsXe_Xpm->dwOutputFrameWidth + pOutSurface->dwSurfaceXOffset - 1;
    cmd.DW7.OutputFrameHeight            = pSfcStateparamsXe_Xpm->dwOutputFrameHeight + pOutSurface->dwSurfaceYOffset - 1;
    cmd.DW8.ScaledRegionSizeWidth        = pSfcStateparamsXe_Xpm->dwScaledRegionWidth - 1;
    cmd.DW8.ScaledRegionSizeHeight       = pSfcStateparamsXe_Xpm->dwScaledRegionHeight - 1;
    cmd.DW9.ScaledRegionHorizontalOffset = pSfcStateparamsXe_Xpm->dwScaledRegionHorizontalOffset + pOutSurface->dwSurfaceXOffset;
    cmd.DW9.ScaledRegionVerticalOffset   = pSfcStateparamsXe_Xpm->dwScaledRegionVerticalOffset + pOutSurface->dwSurfaceYOffset;

    // Set DW10
    cmd.DW10.GrayBarPixelUG              = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateparamsXe_Xpm->fColorFillUGPixel * 1024.0F), 0, 1023); // U10
    cmd.DW10.GrayBarPixelYR              = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateparamsXe_Xpm->fColorFillYRPixel * 1024.0F), 0, 1023); // U10

    // Set DW11
    cmd.DW11.GrayBarPixelA               = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateparamsXe_Xpm->fColorFillAPixel * 1024.0F), 0, 1023); // U10
    cmd.DW11.GrayBarPixelVB              = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateparamsXe_Xpm->fColorFillVBPixel * 1024.0F), 0, 1023); // U10

    // Set DW13
    cmd.DW13.AlphaDefaultValue           = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateparamsXe_Xpm->fAlphaPixel * 1024.0F), 0, 1023); // U10

    // Use round to zero for the scaling factor calculation to resolve tdr issue in scalability case
    // Set DW14
    cmd.DW14.ScalingFactorHeight         = (uint32_t)((double)pSfcStateparamsXe_Xpm->dwSourceRegionHeight / (double)pSfcStateparamsXe_Xpm->dwScaledRegionHeight * 524288.0F); // U4.19

    // Set DW15
    cmd.DW15.ScaleFactorWidth          = (uint32_t)((double)pSfcStateparamsXe_Xpm->dwSourceRegionWidth / (double)pSfcStateparamsXe_Xpm->dwScaledRegionWidth * 524288.0F); // U4.19

    // Set DW19
    if (pSfcStateparamsXe_Xpm->bMMCEnable            &&
       (pSfcStateparamsXe_Xpm->MMCMode == MOS_MMC_RC || 
        pSfcStateparamsXe_Xpm->MMCMode == MOS_MMC_MC))
    {
        cmd.DW19.OutputFrameSurfaceBaseAddressMemoryCompressionEnable = pSfcStateparamsXe_Xpm->bMMCEnable;
        cmd.DW3.OutputCompressionFormat                               = pOutSurface->dwCompressionFormat;
    }

    cmd.DW19.CompressionType = (pSfcStateparamsXe_Xpm->MMCMode == MOS_MMC_RC) ? 1 : 0;

    // Set DW29
    cmd.DW29.OutputSurfaceTiledMode              = MosGetHWTileType(pOutSurface->TileType, pOutSurface->TileModeGMM, pOutSurface->bGMMTileEnabled);
    cmd.DW29.OutputSurfaceHalfPitchForChroma     = bHalfPitchForChroma;
    cmd.DW29.OutputSurfacePitch                  = pOutSurface->dwPitch - 1;
    cmd.DW29.OutputSurfaceInterleaveChromaEnable = bInterleaveChroma;
    cmd.DW29.OutputSurfaceFormat                 = cmd.DW3.OutputSurfaceFormatType;

    // Set DW30, DW31
    cmd.DW30.OutputSurfaceYOffsetForU    = wUYOffset;
    cmd.DW30.OutputSurfaceXOffsetForU    = wUXOffset;
    cmd.DW31.OutputSurfaceYOffsetForV    = wVYOffset;
    cmd.DW31.OutputSurfaceXOffsetForV    = wVXOffset;

    // DW34, DW35
    cmd.DW34.Sourcestartx         = pSfcStateparamsXe_Xpm->srcStartX;
    cmd.DW34.Sourceendx           = pSfcStateparamsXe_Xpm->srcEndX;
    cmd.DW35.Destinationstartx    = pSfcStateparamsXe_Xpm->dstStartX;
    cmd.DW35.Destinationendx      = pSfcStateparamsXe_Xpm->dstEndX;

    // Set DW36, DW37
    //Change SFC outputcentering scaling X/Yphaseshift value and limition limitione with 19bit following Fuslim setting.
    if (m_outputCenteringEnable)
    {
        cmd.DW36.Xphaseshift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)cmd.DW15.ScaleFactorWidth / 524288.0F - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
        cmd.DW37.Yphaseshift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)cmd.DW14.ScalingFactorHeight / 524288.0F - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
    }

    if (pSfcStateparamsXe_Xpm->pOsResOutputSurface)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (pSfcStateparamsXe_Xpm->iScalingType == ISCALING_INTERLEAVED_TO_FIELD &&
            pSfcStateparamsXe_Xpm->outputSampleType == SAMPLE_SINGLE_BOTTOM_FIELD)
        {
            ResourceParams.presResource                = pSfcStateparamsXe_Xpm->tempFieldResource;
            ResourceParams.pdwCmd                      = &(cmd.DW17.Value);
            ResourceParams.dwLocationInCmd             = 17;
            ResourceParams.HwCommandType               = MOS_SFC_STATE;
            ResourceParams.bIsWritable                 = true;
        }
        else
        {
            ResourceParams.presResource                = pSfcStateparamsXe_Xpm->pOsResOutputSurface;
            ResourceParams.pdwCmd                      = &(cmd.DW17.Value);
            ResourceParams.dwLocationInCmd             = 17;
            ResourceParams.HwCommandType               = MOS_SFC_STATE;
            ResourceParams.bIsWritable                 = true;
        }
        ResourceParams.dwOffset                        = pSfcStateparamsXe_Xpm->dwOutputSurfaceOffset;
        InitMocsParams(ResourceParams, &cmd.DW19.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
        if (m_outputSurfCtrl.Gen12_5.Index != 0)
        {
            cmd.DW19.OutputFrameSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables = m_outputSurfCtrl.Gen12_5.Index;
        }
    }

    if (pSfcStateparamsXe_Xpm->pOsResAVSLineBuffer && !Mos_ResourceIsNull(pSfcStateparamsXe_Xpm->pOsResAVSLineBuffer))
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pSfcStateparamsXe_Xpm->pOsResAVSLineBuffer;
        ResourceParams.pdwCmd          = &(cmd.DW20.Value);
        ResourceParams.dwLocationInCmd = 20;
        ResourceParams.HwCommandType   = MOS_SFC_STATE;
        ResourceParams.bIsWritable     = true;
        InitMocsParams(ResourceParams, &cmd.DW22.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
        if (m_avsLineBufferCtrl.Gen12_5.Index != 0)
        {
            cmd.DW22.AvsLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_avsLineBufferCtrl.Gen12_5.Index;
        }
    }

    if (pSfcStateparamsXe_Xpm->pOsResIEFLineBuffer && !Mos_ResourceIsNull(pSfcStateparamsXe_Xpm->pOsResIEFLineBuffer))
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pSfcStateparamsXe_Xpm->pOsResIEFLineBuffer;
        ResourceParams.pdwCmd          = &(cmd.DW23.Value);
        ResourceParams.dwLocationInCmd = 23;
        ResourceParams.HwCommandType   = MOS_SFC_STATE;
        ResourceParams.bIsWritable     = true;
        InitMocsParams(ResourceParams, &cmd.DW25.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
        if (m_iefLineBufferCtrl.Gen12_5.Index != 0)
        {
            cmd.DW25.IefLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_iefLineBufferCtrl.Gen12_5.Index;
        }
    }

    if (pSfcStateparamsXe_Xpm->resSfdLineBuffer && !Mos_ResourceIsNull(pSfcStateparamsXe_Xpm->resSfdLineBuffer))
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource = pSfcStateparamsXe_Xpm->resSfdLineBuffer;
        ResourceParams.pdwCmd = &(cmd.DW26.Value);
        ResourceParams.dwLocationInCmd = 26;
        ResourceParams.HwCommandType = MOS_SFC_STATE;
        ResourceParams.bIsWritable = true;
        InitMocsParams(ResourceParams, &cmd.DW28.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
        if (m_sfdLineBufferCtrl.Gen12_5.Index != 0)
        {
            cmd.DW28.SfdLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_sfdLineBufferCtrl.Gen12_5.Index;
        }
    }

    if (pSfcStateparamsXe_Xpm->resAvsLineTileBuffer && !Mos_ResourceIsNull(pSfcStateparamsXe_Xpm->resAvsLineTileBuffer))
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource = pSfcStateparamsXe_Xpm->resAvsLineTileBuffer;
        ResourceParams.pdwCmd = &(cmd.DW38.Value);
        ResourceParams.dwLocationInCmd = 38;
        ResourceParams.HwCommandType = MOS_SFC_STATE;
        ResourceParams.bIsWritable = true;
        InitMocsParams(ResourceParams, &cmd.DW40.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
        if (m_avsLineTileBufferCtrl.Gen12_5.Index != 0)
        {
            cmd.DW40.AvsLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_avsLineTileBufferCtrl.Gen12_5.Index;
        }
    }

    if (pSfcStateparamsXe_Xpm->resIefLineTileBuffer && !Mos_ResourceIsNull(pSfcStateparamsXe_Xpm->resIefLineTileBuffer))
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource = pSfcStateparamsXe_Xpm->resIefLineTileBuffer;
        ResourceParams.pdwCmd = &(cmd.DW41.Value);
        ResourceParams.dwLocationInCmd = 41;
        ResourceParams.HwCommandType = MOS_SFC_STATE;
        ResourceParams.bIsWritable = true;
        InitMocsParams(ResourceParams, &cmd.DW43.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
        if (m_iefLineTileBufferCtrl.Gen12_5.Index != 0)
        {
            cmd.DW43.IefLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_iefLineTileBufferCtrl.Gen12_5.Index;
        }
    }

    if (pSfcStateparamsXe_Xpm->resSfdLineTileBuffer && !Mos_ResourceIsNull(pSfcStateparamsXe_Xpm->resSfdLineTileBuffer))
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource = pSfcStateparamsXe_Xpm->resSfdLineTileBuffer;
        ResourceParams.pdwCmd = &(cmd.DW44.Value);
        ResourceParams.dwLocationInCmd = 44;
        ResourceParams.HwCommandType = MOS_SFC_STATE;
        ResourceParams.bIsWritable = true;
        InitMocsParams(ResourceParams, &cmd.DW46.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
        if (m_sfdLineTileBufferCtrl.Gen12_5.Index != 0)
        {
            cmd.DW46.SfdLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables = m_sfdLineTileBufferCtrl.Gen12_5.Index;
        }
    }

    if (pSfcStateparamsXe_Xpm->histogramSurface && !Mos_ResourceIsNull(&pSfcStateparamsXe_Xpm->histogramSurface->OsResource))
    {
        cmd.DW4.HistogramStreamout                 = 1;
        cmd.DW49.Value                             = 0;

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource                = &pSfcStateparamsXe_Xpm->histogramSurface->OsResource;
        ResourceParams.dwOffset                    = pSfcStateparamsXe_Xpm->histogramSurface->dwOffset;
        ResourceParams.pdwCmd                      = &(cmd.DW47.Value);
        ResourceParams.dwLocationInCmd             = 47;
        ResourceParams.HwCommandType               = MOS_SFC_STATE;
        ResourceParams.bIsWritable                 = true;
        InitMocsParams(ResourceParams, &cmd.DW49.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
        if (m_histogramBufferCtrl.Gen12_5.Index != 0)
        {
            cmd.DW49.HisgotramBaseAddressIndexToMemoryObjectControlStateMocsTables = m_histogramBufferCtrl.Gen12_5.Index;
        }
    }

    // Input/Output frame format
    cmd.DW1.InputFrameDataFormat  = pSfcStateparamsXe_Xpm->inputFrameDataFormat;
    cmd.DW1.OutputFrameDataFormat = pSfcStateparamsXe_Xpm->outputFrameDataFormat;

    // interleaved to interleaved
    if (pSfcStateparamsXe_Xpm->iScalingType == ISCALING_INTERLEAVED_TO_INTERLEAVED)
    {
        cmd.DW54.BottomFieldVerticalScalingOffset = pSfcStateparamsXe_Xpm->bottomFieldVerticalScalingOffset;
    }

    // Input is field mode
    if (pSfcStateparamsXe_Xpm->iScalingType == ISCALING_FIELD_TO_INTERLEAVED)
    {
        cmd.DW1.TopBottomField      = pSfcStateparamsXe_Xpm->topBottomField;
        cmd.DW1.TopBottomFieldFirst = pSfcStateparamsXe_Xpm->topBottomFieldFirst;
    }

    // interleaved to field
    if (pSfcStateparamsXe_Xpm->iScalingType == ISCALING_INTERLEAVED_TO_FIELD)
    {
        // Add bottom field address to cmd
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (pSfcStateparamsXe_Xpm->outputSampleType == SAMPLE_SINGLE_BOTTOM_FIELD)
        {
            ResourceParams.dwLsbNum        = 12;
            ResourceParams.presResource    = pSfcStateparamsXe_Xpm->pOsResOutputSurface;
            ResourceParams.pdwCmd          = &cmd.DW55.Value;
            ResourceParams.dwLocationInCmd = 55;
            ResourceParams.bIsWritable     = true;
        }
        else
        {
            ResourceParams.dwLsbNum        = 12;
            ResourceParams.presResource    = pSfcStateparamsXe_Xpm->tempFieldResource;
            ResourceParams.pdwCmd          = &cmd.DW55.Value;
            ResourceParams.dwLocationInCmd = 55;
            ResourceParams.bIsWritable     = true;
        }
        InitMocsParams(ResourceParams, &cmd.DW57.Value, 1, 6);
        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
            m_osInterface,
            pCmdBuffer,
            &ResourceParams));

        // copy from base surface
        cmd.DW57.BottomFieldSurfaceBaseAddressArbitrationPriorityControl =
            cmd.DW19.OutputFrameSurfaceBaseAddressArbitrationPriorityControl;
        cmd.DW57.BottomFieldSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables =
            cmd.DW19.OutputFrameSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables;
        cmd.DW57.BottomFiledSurfaceBaseAddressMemoryCompressionEnable =
            cmd.DW19.OutputFrameSurfaceBaseAddressMemoryCompressionEnable;
        cmd.DW57.BottomFiledSurfaceBaseAddressMemoryCompressionType =
            cmd.DW19.CompressionType;

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
    cmd.DW3.DitherEnable         = pSfcStateparamsXe_Xpm->ditheringEn;
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

        cmd.DW52.DitheringLutDelta4  = 0;
        cmd.DW52.DitheringLutDelta5  = 1;
        cmd.DW52.DitheringLutDelta6  = 0;
        cmd.DW52.DitheringLutDelta7  = 0;

        cmd.DW53.DitheringLutDelta0  = 0;
        cmd.DW53.DitheringLutDelta1  = 0;
        cmd.DW53.DitheringLutDelta2  = 0;
        cmd.DW53.DitheringLutDelta3  = 0;
    }

    if (m_sfcScalabilityEnabled == true)
    {
        uint32_t        iMediumX;
        uint32_t        Xoffset;
        uint32_t        src_startX[MHW_SFC_MAX_PIPE_NUM_XE_XPM];
        uint32_t        src_endX[MHW_SFC_MAX_PIPE_NUM_XE_XPM];
        uint32_t        dest_startX[MHW_SFC_MAX_PIPE_NUM_XE_XPM];
        uint32_t        dest_endX[MHW_SFC_MAX_PIPE_NUM_XE_XPM];
        uint32_t        tile_endX;
        uint32_t        dest_first_valid_left_tile = 0;
        uint32_t        dest_last_valid_right_tile = m_numofSfc - 1;
        uint32_t        dest_cntX = 0;
        double          Xlandingpoint;
        uint32_t        one_by_sf = (uint32_t)(((uint64_t)pSfcStateparamsXe_Xpm->dwSourceRegionWidth * 524288L)/ pSfcStateparamsXe_Xpm->dwScaledRegionWidth);
        const uint32_t  one_by_sf_fraction_precision = 19;
        const uint32_t  beta_precision = 5;
        int32_t         xPhaseShift;
        double          tempDestCntx;
        uint32_t        i;
        MHW_ASSERT(pSfcStateparamsXe_Xpm->dwInputFrameWidth > m_numofSfc * 64);

        iMediumX = MOS_ALIGN_FLOOR((pSfcStateparamsXe_Xpm->dwInputFrameWidth / m_numofSfc), 64);
        iMediumX = MOS_CLAMP_MIN_MAX(iMediumX, 64, (pSfcStateparamsXe_Xpm->dwInputFrameWidth - 64));

        src_startX[0] = dest_startX[0] = 0;

        for (i = 0; i < m_numofSfc; i++)
        {
            if (i == m_numofSfc - 1)
            {
                src_endX[i] =  pSfcStateparamsXe_Xpm->dwInputFrameWidth - 1;
            }
            else
            {
                src_endX[i]  = iMediumX * (i + 1) - 1;
                if (pSfcStateparamsXe_Xpm->dwInputFrameWidth != m_numofSfc * iMediumX)
                {
                    src_endX[i] += 64;
                }
            }

            if (pSfcStateparamsXe_Xpm->dwSourceRegionHorizontalOffset + pSfcStateparamsXe_Xpm->dwSourceRegionWidth - 1 <= src_endX[i])
            {
                Xoffset   = 0;
                tile_endX = pSfcStateparamsXe_Xpm->dwSourceRegionHorizontalOffset + pSfcStateparamsXe_Xpm->dwSourceRegionWidth - 1;
            }
            else
            {
                // Fix tdr issue with odd width output, the Xoffset was set to VALUE_XOFFSET always for vesfc workload
                Xoffset   = VALUE_XOFFSET;
                tile_endX = src_endX[i];
            }

            while (1)
            {
                if (src_endX[i] - pSfcStateparamsXe_Xpm->dwSourceRegionHorizontalOffset < (Xoffset + 1))
                {
                    dest_endX[i] = 0;
                    break;
                }

                if (dest_cntX == 0)
                {
                    dest_first_valid_left_tile = i;
                }

                xPhaseShift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)pSfcStateparamsXe_Xpm->dwSourceRegionWidth / pSfcStateparamsXe_Xpm->dwScaledRegionWidth - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));
                tempDestCntx = (((double)dest_cntX * (double)one_by_sf) + xPhaseShift);
                if (tempDestCntx < 0)
                {
                    tempDestCntx = 0;
                }
                Xlandingpoint = (double)(((tempDestCntx + ((double)(1 << (one_by_sf_fraction_precision - beta_precision - 1)))) / 524288.0F) + pSfcStateparamsXe_Xpm->dwSourceRegionHorizontalOffset);

                if (Xlandingpoint >= (double)(tile_endX - Xoffset))
                {
                    dest_endX[i] = dest_cntX - 1;
                    break;
                }
                else
                {
                    dest_cntX ++;
                }

                if (Xoffset == 0)
                {
                    dest_last_valid_right_tile = i;
                    dest_endX[i] = pSfcStateparamsXe_Xpm->dwScaledRegionWidth - 1;
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
                dest_startX[i] = dest_endX[i-1] + 1;
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

        if (pSfcStateparamsXe_Xpm->bIEFEnable)
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

        if (VpHalDDIUtils::GetSurfaceColorPack(pSfcStateparamsXe_Xpm->OutputFrameFormat) != VPHAL_COLORPACK_444 &&
            (dest_startX[m_indexofSfc] %2 != 0))
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

        if (pSfcStateparamsXe_Xpm->pOsResAVSLineBufferSplit[m_indexofSfc] && !Mos_ResourceIsNull(pSfcStateparamsXe_Xpm->pOsResAVSLineBufferSplit[m_indexofSfc]))
        {
            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            ResourceParams.presResource    = pSfcStateparamsXe_Xpm->pOsResAVSLineBufferSplit[m_indexofSfc];
            ResourceParams.pdwCmd          = &(cmd.DW20.Value);
            ResourceParams.dwLocationInCmd = 20;
            ResourceParams.HwCommandType   = MOS_SFC_STATE;
            ResourceParams.bIsWritable     = true;

            MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
                pOsInterface,
                pCmdBuffer,
                &ResourceParams));
        }

        if (pSfcStateparamsXe_Xpm->pOsResIEFLineBufferSplit[m_indexofSfc] && !Mos_ResourceIsNull(pSfcStateparamsXe_Xpm->pOsResIEFLineBufferSplit[m_indexofSfc]))
        {
            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            ResourceParams.presResource    = pSfcStateparamsXe_Xpm->pOsResIEFLineBufferSplit[m_indexofSfc];
            ResourceParams.pdwCmd          = &(cmd.DW23.Value);
            ResourceParams.dwLocationInCmd = 23;
            ResourceParams.HwCommandType   = MOS_SFC_STATE;
            ResourceParams.bIsWritable     = true;

            MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
                pOsInterface,
                pCmdBuffer,
                &ResourceParams));
        }

        MHW_NORMALMESSAGE("SFC%d STATE: src startx %d endx %d", m_indexofSfc, cmd.DW34.Sourcestartx, cmd.DW34.Sourceendx);
        MHW_NORMALMESSAGE("SFC%d STATE: dest startx %d endx %d", m_indexofSfc, cmd.DW35.Destinationstartx, cmd.DW35.Destinationendx);
    }

    MHW_CHK_STATUS_RETURN(pOsInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Set which Sfc can be used by HW
//! \details  VPHAL set which Sfc can be use by HW
//! \param    [in] dwSfcIndex;
//!           set which Sfc can be used by HW
//! \param    [in] dwSfcCount;
//!           set Sfc Count
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
MOS_STATUS MhwSfcInterfaceXe_Xpm::SetSfcIndex(
    uint32_t dwSfcIndex,
    uint32_t dwSfcCount)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_ASSERT(dwSfcIndex < dwSfcCount);

    m_indexofSfc            = dwSfcIndex;
    m_numofSfc              = dwSfcCount;
    m_sfcScalabilityEnabled = (dwSfcCount > 1) ? true : false;

    return eStatus;
}
